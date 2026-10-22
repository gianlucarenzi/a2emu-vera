/* memtest.c -- the IIe's 64 KB aux RAM next to the A2VERA card, and the VERA's VRAM
 * (the Apple II counterpart of VERA_C64/tests/test_mem.c).
 *
 * Booted by sdk/boot.s, no DOS; the VERA card is found in any slot.
 *  1. aux RAM $0200-$BFFF (190 pages; $0000-$01FF and the aux language card
 *     are not tested: they switch with the stack): every page is filled in
 *     main RAM, moved to aux with the IIe's AUXMOVE ($C311), moved back and
 *     compared; VERA registers are written and read back after every page.
 *     Skipped on a ][+ or a IIe without the extended 80-column card (A2_AUX=64).
 *  2. VRAM $00000-$1F9BF (below the PSG, palette and sprite registers) through
 *     DATA0, increment 1: filled, then verified.
 * Every byte tested also goes straight into the VERA border colour register (the
 * border covers the whole screen: VSTART = VSTOP); at the end the border shows the
 * result: green = ok, red = error.  main never returns. */

#include <conio.h>
#include <apple2.h>
#include "vera.h"

#define OK_COLOR  5                     /* green in the VERA default palette */
#define ERR_COLOR 2                     /* red */

#define AUX_FIRST 0x02                  /* aux pages tested */
#define AUX_LAST  0xBF

#define VRAM_PAGES 0x1F9u               /* full pages below $1F9C0 */
#define VRAM_TAIL  0xC0u                /* + $1F900-$1F9BF */

#define PATTERN(h, l, i) ((unsigned char)((l) ^ ((h) * 3) ^ (i) ^ 0x5A))

static unsigned char buf[256];

/* AUXMOVE: copy src..src+255 (main -> aux if to_aux, else aux -> main) to dst */
static void auxmove(unsigned src, unsigned dst, unsigned char to_aux)
{
    *(unsigned *)0x3C = src;            /* A1: start */
    *(unsigned *)0x3E = src + 255;      /* A2: end */
    *(unsigned *)0x42 = dst;            /* A4: destination */
    if (to_aux)
        __asm__("sec");
    else
        __asm__("clc");
    __asm__("jsr $C311");
}

/* is there RAM in aux at page p? */
static unsigned char aux_present(void)
{
    unsigned char i = 0;

    do { buf[i] = (unsigned char)(i ^ 0xC3); } while (++i);
    auxmove((unsigned)buf, 0x0800, 1);
    do { buf[i] = 0; } while (++i);
    auxmove(0x0800, (unsigned)buf, 0);
    do { if (buf[i] != (unsigned char)(i ^ 0xC3)) return 0; } while (++i);
    return 1;
}

int main(void)
{
    register volatile unsigned char *data0, *border;
    unsigned char p, i, k, aux, bad_vreg = 0;
    unsigned int h, bad_aux = 0, bad_vram = 0;

    clrscr();
    if (!vera_detect()) {
        cputs("VERA card not found in slots 1-7\r\n");
        for (;;)
            ;
    }
    cprintf("AUX RAM + VERA (slot %u) TEST\r\n", vera_slot);
    data0 = &VERA_DATA0;
    border = &VERA_DC_BORDER;

    VERA_CTRL      = 0x02;              /* DCSEL 1: no active area, all border */
    VERA_DC_VSTART = 0;
    VERA_DC_VSTOP  = 0;
    VERA_CTRL      = 0;
    VERA_DC_VIDEO  = 0x01;              /* VGA, no layers */

    aux = (get_ostype() & 0xF0) == (APPLE_IIE & 0xF0) && aux_present();
    if (aux) {
        cputs("aux $0200-$BFFF...");
        for (p = AUX_FIRST; p <= AUX_LAST; ++p) {
            i = 0;
            do { *border = buf[i] = PATTERN(0, p, i); } while (++i);
            auxmove((unsigned)buf, (unsigned)p << 8, 1);
            VERA_ADDR_L = p;            /* ADDR0_L / ADDR0_M */
            VERA_ADDR_M = (unsigned char)~p;
            if (VERA_ADDR_L != p || VERA_ADDR_M != (unsigned char)~p) ++bad_vreg;
        }
        for (p = AUX_FIRST; p <= AUX_LAST; ++p) {
            auxmove((unsigned)p << 8, (unsigned)buf, 0);
            i = 0;
            do { if ((*border = buf[i]) != PATTERN(0, p, i)) ++bad_aux; } while (++i);
        }
        cputs(bad_aux ? " ERROR\r\n" : " OK\r\n");
    } else
        cputs("no aux RAM (IIe + A2_AUX=64): skipped\r\n");

    cputs("VRAM $00000-$1F9BF...");
    VERA_SET_ADDR(0x00000ul, VERA_INC1);
    for (h = 0; h <= VRAM_PAGES; ++h) {
        k = PATTERN(h >> 8, h, 0);
        i = 0;
        if (h < VRAM_PAGES)
            do { *border = *data0 = k ^ i; } while (++i);
        else
            for (; i < VRAM_TAIL; ++i) *border = *data0 = k ^ i;
    }
    VERA_SET_ADDR(0x00000ul, VERA_INC1);
    for (h = 0; h <= VRAM_PAGES; ++h) {
        k = PATTERN(h >> 8, h, 0);
        i = 0;
        if (h < VRAM_PAGES)
            do { if ((*border = *data0) != (k ^ i)) ++bad_vram; } while (++i);
        else
            for (; i < VRAM_TAIL; ++i) if ((*border = *data0) != (k ^ i)) ++bad_vram;
    }
    cputs(bad_vram ? " ERROR\r\n" : " OK\r\n");

    if (aux)
        cprintf("AUX RAM:   %s (%u bad)\r\n", bad_aux ? "ERROR" : "OK", bad_aux);
    cprintf("VERA REGS: %s\r\n", bad_vreg ? "ERROR" : "OK");
    cprintf("VERA VRAM: %s (%u bad)\r\n", bad_vram ? "ERROR" : "OK", bad_vram);
    *border = (bad_aux | bad_vreg | bad_vram) ? ERR_COLOR : OK_COLOR;
    for (;;)
        ;
}
