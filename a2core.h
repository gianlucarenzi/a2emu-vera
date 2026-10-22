/* a2core.h -- the Apple II+ machine (a2core.c) as seen by a frontend
 * (reinette.c = SDL, a2run.c = headless).  puce6502.h must come first
 * (it defines bool/true/false for this code base). */

#ifndef A2CORE_H
#define A2CORE_H

#include <stdint.h>
#include <stdio.h>
#include "puce6502.h"
#include "veracore.h"

// memory layout
#define RAMSIZE  0xC000
#define ROMSTART 0xD000
#define ROMSIZE  0x3000
#define LGCSTART 0xD000
#define LGCSIZE  0x3000
#define BK2START 0xD000
#define BK2SIZE  0x1000
#define SL6START 0xC600
#define SL6SIZE  0x0100
#define SL7START 0xC700

extern uint8_t ram[RAMSIZE], rom[ROMSIZE], lgc[LGCSIZE], bk2[BK2SIZE], sl6[SL6SIZE];
extern uint8_t fetchMem[0x10000];
void fetchSyncAll(void);
void fetchSyncRam(void);
void fetchSyncCx(void);
void fetchSyncHigh(void);

// soft switches / keyboard
extern uint8_t KBD;
extern int     g_akd;
extern uint8_t g_kbdDown;
extern bool TEXT, MIXED, PAGE2, HIRES, LCWR, LCRD, LCBK2, LCWFF;

// game I/O
extern uint8_t PB0, PB1, PB2;
extern uint8_t g_gpad;
extern int     g_gpad_en;
extern float   GCP[4], GCC[4];          // paddles 0-3 (0..255, < 0 = not connected)
extern int     GCD[2], GCA[2];
extern uint8_t GCActionSpeed, GCReleaseSpeed;
// SNES MAX card (A2_SNESMAX env, a2run --snesmax): bits = the pads' shift registers,
// ACTIVE LOW (0xFFFF = nothing pressed), MSB first = B Y Sel St U D L R A X L R 1 1 1 1
extern int      g_snesmax_slot;
extern uint16_t g_snesmax_bits, g_snesmax_p2;

// Disk ][
struct drive {
	char     filename[400];                 // the full disk image pathname
	bool     readOnly;                      // based on the image file attributes
	uint8_t  data[232960];                  // nibblelized disk image
	bool     motorOn;                       // motor status
	bool     writeMode;                     // writes to file are not implemented
	uint8_t  track;                         // current track position
	uint16_t nibble;                        // ptr to nibble under head position
	int      fmt;                           // DISK_DOS / DISK_PO / DISK_NIB (the file's format)
	bool     dirty;                         // written since insert/flush (E6 write-back)
};
enum { DISK_DOS = 0, DISK_PO = 1, DISK_NIB = 2 };
extern struct drive disk[2];
extern int curDrv;
extern int g_disk_dbg;
int  insertFloppy(void *wdo, char *filename, int drv);
int  saveFloppy(int drive);
int  insertHardDisk(char *filename);       // slot-7 ProDOS block device (.po/.hdv/.2mg)
extern uint8_t sl7[256];
extern int     g_hd_present;
int  disk_motor_on(void);
int  disk_fmt_of(const char *filename);
int  denibblize(const uint8_t *nib, uint8_t *dsk);   // nibbles -> DOS-order sectors decoded
int  a2_flush_disks(void);                  // write dirty floppies back (E6); 0 = a write failed

// snapshots
int reinetteSaveState(const char *path);
int reinetteLoadState(const char *path);

// VERA card
extern vc_t g_vera;
extern int  g_vera_enabled;
extern int  g_vera_irq;                     // VERA IRQ -> 6502 /IRQ (A2VERA D1), default on
void vera_bus_init(void);
extern uint16_t g_vera_base;                // $Cn00 of the VERA card (A2_VERA_SLOT)
void vera_slot_check(void);                 // warn when the slot is taken by the Disk II / hard disk
void cpu_vera_step(void);                   // ~32 CPU cycles + VERA (59.52 Hz) + IRQ line

// RAM card (Slinky, E5) and debug card (E7)
extern int      g_xmem_slot;
extern uint32_t g_xmem_size, g_xmem_addr;
extern uint8_t *g_xmem;
extern unsigned long g_xmem_reads, g_xmem_writes;
int  xmem_init(unsigned kb, int slot);      // kb = power of two; 0 on error
extern int      g_dbg_slot, g_dbg_exit;
extern void (*a2_dbg_hook)(int reg, uint8_t value);

extern bool muted;                          // speaker mute (frontend toggles, snapshots keep it)

// Apple IIe mode (E13)
extern int  g_iie;                          // 0 = ][+, 1 = IIe, 2 = enhanced IIe
extern int  g_iie_aux;                      // aux 64 KB banks: 0 none, 1 80-col card, 2-128 RamWorks ($C073)
extern int  g_memslow;
extern bool S80STORE, RAMRD, RAMWRT, ALTZP, INTCXROM, SLOTC3ROM, S80COL, ALTCHAR;
extern bool DHIRES;                         // IIe AN3 off ($C05E): double-res when 80COL + aux
int  a2_load_iie_roms(const char *dir, int enhanced);   // 1 = loaded, IIe mode on
int  a2_load_iie_video_rom(const char *dir, int enhanced, uint8_t *buf);   // 2 KB charset, 1 = loaded
const uint8_t *a2_aux_ram(void);            // aux bank 0 (the video one), NULL without aux memory
int  a2_set_aux(const char *s);             // aux memory 0|64K|512K|1M|4M|8M|RAM2E; 0 = invalid
extern int g_ram2e;                         // 1 = the aux card is a RAM2E II
void a2_ram2e_nvm(const char *path);        // RAM2E settings file (NULL = not saved): loads it
void a2_reset(void);                        // machine reset (IIe: switches + LC to ROM)

// frontend hooks (NULL = not provided)
extern void (*a2_speaker_hook)(void);                           // $C030 toggle
extern void (*a2_title_hook)(void *wdo, const char *title);     // disk inserted
const char *a2_model_name(void);            // "Reinette ][+" / "Reinette //e" / "... (enhanced)"

#endif
