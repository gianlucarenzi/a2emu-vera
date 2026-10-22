/* a2run_input.c -- scripted input for a2run (plan E8, WP1.4): `a2run --input FILE`.
 *
 * One event per line:   <vera-frame> <device> <value>      ('#' starts a comment)
 * Events of frame N are applied when VERA frame N has completed (frame 0 = before the
 * CPU runs), in file order; the frames must not decrease.  The state set by an event
 * holds until another event changes it.
 *
 *   key V     a key goes down: KBD = V | $80 (strobe) and, while held, the IIe
 *             any-key-down level ($C010 bit 7, g_kbdDown).  V = one character, $hh,
 *             or a name: SPACE RET ESC TAB LEFT RIGHT UP DOWN DEL
 *   keyup -   all keys up (any-key-down level off; the strobe is left alone)
 *   tap V     strobe only (a key pressed and released between two frames)
 *   pb0 0|1   push buttons 0-2 ($C061-$C063; Open-Apple / Solid-Apple on a IIe)
 *   pb1 0|1   (pb2 is the game-port pad's DATA line while a gpad is connected)
 *   pb2 0|1
 *   pdl0 N    paddle 0-3 position 0..255 (joystick 1 = pdl0 X / pdl1 Y, joystick 2 =
 *   ..pdl3    pdl2/pdl3); `none` = nothing connected (the timer never runs out).
 *             Defaults: pdl0/pdl1 127 (a centred stick), pdl2/pdl3 none
 *   gpad N    game-port NES/SNES pad (CD4021, a2vera INPUT.md wiring) connected with
 *             buttons N pressed: bit7 A, bit6 B, bit5 Select, bit4 Start, bit3 Up,
 *             bit2 Down, bit1 Left, bit0 Right (1 = pressed, first bit shifted out
 *             = bit 7); `gpad off` disconnects it (PB2 is a plain button again)
 *   snes1 N   SNES MAX card pad 1 / pad 2, 16-bit PRESSED mask (1 = pressed), MSB
 *   snes2 N   first: $8000 B, $4000 Y, $2000 Select, $1000 Start, $0800 Up, $0400
 *             Down, $0200 Left, $0100 Right, $0080 A, $0040 X, $0020 L, $0010 R
 *             (needs the card: a2run --snesmax SLOT or A2_SNESMAX)
 * Numbers: decimal, $hex or 0xhex.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "puce6502.h"
#include "a2core.h"
#include "a2run_input.h"

enum { EV_KEY, EV_KEYUP, EV_TAP, EV_PB, EV_PDL, EV_GPAD, EV_SNES };
typedef struct { long frame; int kind, idx, val; } ev_t;

static ev_t *evs = NULL;
static int nev = 0, cap = 0, next = 0;

static int num(const char *s, int *out) {
	char *end; long v;
	if (*s == '$') v = strtol(s + 1, &end, 16);
	else v = strtol(s, &end, 0);
	if (end == s || *end) return 0;
	*out = (int)v;
	return 1;
}

static int keyval(const char *s, int *out) {
	static const struct { const char *n; int v; } names[] = {
		{ "SPACE", 0x20 }, { "RET", 0x0D }, { "ESC", 0x1B }, { "TAB", 0x09 }, { "LEFT", 0x08 },
		{ "RIGHT", 0x15 }, { "UP", 0x0B }, { "DOWN", 0x0A }, { "DEL", 0x7F } };
	for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
		if (!strcmp(s, names[i].n)) { *out = names[i].v; return 1; }
	if (s[0] && !s[1]) { *out = (unsigned char)s[0] & 0x7F; return 1; }
	return num(s, out) && *out >= 0 && *out < 0x80;
}

int a2in_load(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) { fprintf(stderr, "a2run: cannot open input script %s\n", path); return 0; }
	char line[256]; int ln = 0; long last = 0;
	while (fgets(line, sizeof line, f)) {
		ln++;
		char *h = strchr(line, '#'); if (h) *h = 0;
		char a[64], d[64], v[64];
		int n = sscanf(line, "%63s %63s %63s", a, d, v);
		if (n <= 0) continue;
		ev_t e = { 0, 0, 0, 0 }; int fr;
		if (n < 2 || !num(a, &fr) || fr < 0) goto bad;
		e.frame = fr;
		if (e.frame < last) { fprintf(stderr, "a2run: %s:%d: frames must not decrease\n", path, ln); fclose(f); return 0; }
		last = e.frame;
		if (!strcmp(d, "keyup")) e.kind = EV_KEYUP;
		else if (n < 3) goto bad;
		else if (!strcmp(d, "key") || !strcmp(d, "tap")) {
			e.kind = d[0] == 'k' ? EV_KEY : EV_TAP;
			if (!keyval(v, &e.val)) goto bad;
		} else if (!strncmp(d, "pb", 2) && d[2] >= '0' && d[2] <= '2' && !d[3]) {
			e.kind = EV_PB; e.idx = d[2] - '0';
			if (!num(v, &e.val)) goto bad;
		} else if (!strncmp(d, "pdl", 3) && d[3] >= '0' && d[3] <= '3' && !d[4]) {
			e.kind = EV_PDL; e.idx = d[3] - '0';
			if (!strcmp(v, "none")) e.val = -1;
			else if (!num(v, &e.val) || e.val < 0 || e.val > 255) goto bad;
		} else if (!strcmp(d, "gpad")) {
			e.kind = EV_GPAD;
			if (!strcmp(v, "off")) e.val = -1;
			else if (!num(v, &e.val) || e.val < 0 || e.val > 255) goto bad;
		} else if (!strcmp(d, "snes1") || !strcmp(d, "snes2")) {
			e.kind = EV_SNES; e.idx = d[4] - '1';
			if (!num(v, &e.val) || e.val < 0 || e.val > 0xFFFF) goto bad;
		} else goto bad;
		if (nev == cap) { cap = cap ? cap * 2 : 64; evs = realloc(evs, cap * sizeof *evs); }
		evs[nev++] = e;
		continue;
bad:
		fprintf(stderr, "a2run: %s:%d: bad input event (see a2run_input.c)\n", path, ln);
		fclose(f);
		return 0;
	}
	fclose(f);
	return 1;
}

void a2in_frame(long frame) {
	while (next < nev && evs[next].frame <= frame) {
		const ev_t *e = &evs[next++];
		switch (e->kind) {
		case EV_KEY:   KBD = (uint8_t)(e->val | 0x80); g_kbdDown = 0x80; break;
		case EV_KEYUP: g_kbdDown = 0; break;
		case EV_TAP:   KBD = (uint8_t)(e->val | 0x80); g_kbdDown = 0; break;
		case EV_PB: {
			uint8_t v = e->val ? 0x80 : 0x00;
			if (e->idx == 0) PB0 = v; else if (e->idx == 1) PB1 = v; else PB2 = v;
			break; }
		case EV_PDL:   GCP[e->idx] = (float)e->val; break;
		case EV_GPAD:
			if (e->val < 0) g_gpad_en = 0;
			else { g_gpad_en = 1; g_gpad = (uint8_t)e->val; }
			break;
		case EV_SNES:
			if (!g_snesmax_slot) fprintf(stderr, "a2run: snes%d event without a SNES MAX card (--snesmax)\n", e->idx + 1);
			if (e->idx == 0) g_snesmax_bits = (uint16_t)~e->val; else g_snesmax_p2 = (uint16_t)~e->val;
			break;
		}
	}
}
