/* vera.h -- the A2VERA card's registers for cc65 programs (-t apple2).
 *
 * The card decodes $Cn00-$Cn1F of its slot n; vera_detect() finds the slot at
 * run time and points `vera` at it, so the same program works in any slot:
 *
 *     if (!vera_detect()) ... no card
 *     VERA_CTRL = 0;  VERA_DC_VIDEO = 0x11;  ...
 *
 * Register layout as in the VERA 48.0.1 HDL (DCSEL = CTRL bits 6:1). */

#ifndef VERA_H
#define VERA_H

/* $Cn00 of the card; set by vera_detect() */
extern volatile unsigned char *vera;
/* its slot, 0 = not found */
extern unsigned char vera_slot;

#define VERA_REG(o)     (vera[o])

#define VERA_ADDR_L     VERA_REG(0x00)
#define VERA_ADDR_M     VERA_REG(0x01)
#define VERA_ADDR_H     VERA_REG(0x02)  /* bits 7:3 increment, bit 0 VRAM bit 16 */
#define VERA_DATA0      VERA_REG(0x03)
#define VERA_DATA1      VERA_REG(0x04)
#define VERA_CTRL       VERA_REG(0x05)  /* bit 0 ADDRSEL, bits 6:1 DCSEL */
#define VERA_IEN        VERA_REG(0x06)
#define VERA_ISR        VERA_REG(0x07)  /* write 1s to clear VSYNC/LINE/SPRCOL */
#define VERA_IRQLINE_L  VERA_REG(0x08)
#define VERA_DC_VIDEO   VERA_REG(0x09)  /* DCSEL 0 */
#define VERA_DC_HSCALE  VERA_REG(0x0A)
#define VERA_DC_VSCALE  VERA_REG(0x0B)
#define VERA_DC_BORDER  VERA_REG(0x0C)
#define VERA_DC_HSTART  VERA_REG(0x09)  /* DCSEL 1 */
#define VERA_DC_HSTOP   VERA_REG(0x0A)
#define VERA_DC_VSTART  VERA_REG(0x0B)
#define VERA_DC_VSTOP   VERA_REG(0x0C)
#define VERA_REG09      VERA_REG(0x09)  /* DCSEL 63: 'V', major, minor, patch */
#define VERA_REG0A      VERA_REG(0x0A)
#define VERA_REG0B      VERA_REG(0x0B)
#define VERA_REG0C      VERA_REG(0x0C)
#define VERA_L0_CONFIG   VERA_REG(0x0D)
#define VERA_L0_MAPBASE  VERA_REG(0x0E)
#define VERA_L0_TILEBASE VERA_REG(0x0F)
#define VERA_L0_HSCR_L   VERA_REG(0x10)
#define VERA_L0_HSCR_H   VERA_REG(0x11)
#define VERA_L0_VSCR_L   VERA_REG(0x12)
#define VERA_L0_VSCR_H   VERA_REG(0x13)
#define VERA_L1_CONFIG   VERA_REG(0x14)
#define VERA_L1_MAPBASE  VERA_REG(0x15)
#define VERA_L1_TILEBASE VERA_REG(0x16)
#define VERA_L1_HSCR_L   VERA_REG(0x17)
#define VERA_L1_HSCR_H   VERA_REG(0x18)
#define VERA_L1_VSCR_L   VERA_REG(0x19)
#define VERA_L1_VSCR_H   VERA_REG(0x1A)
#define VERA_AUDIO_CTRL VERA_REG(0x1B)
#define VERA_AUDIO_RATE VERA_REG(0x1C)
#define VERA_AUDIO_DATA VERA_REG(0x1D)
#define VERA_SPI_DATA   VERA_REG(0x1E)
#define VERA_SPI_CTRL   VERA_REG(0x1F)

#define VERA_ISR_VSYNC  0x01
#define VERA_ISR_LINE   0x02

/* ADDR_H increment field */
#define VERA_INC0       0x00
#define VERA_INC1       0x10
#define VERA_INC2       0x20
#define VERA_INC4       0x30

#define VERA_PSG_BASE     0x1F9C0ul     /* 16 voices x 4 bytes */
#define VERA_PALETTE_BASE 0x1FA00ul     /* 256 x 2 bytes, GGGGBBBB 0000RRRR */
#define VERA_SPRITE_BASE  0x1FC00ul     /* 128 x 8 bytes */

/* Point data port 0 at a VRAM address with the given increment. */
#define VERA_SET_ADDR(addr, inc) do { \
        VERA_CTRL = 0; \
        VERA_ADDR_L = (unsigned char)(addr); \
        VERA_ADDR_M = (unsigned char)((addr) >> 8); \
        VERA_ADDR_H = (unsigned char)((inc) | (((addr) >> 16) & 1)); \
    } while (0)

/* Find the card: for slots 1-7, skip the ones with disk-controller firmware
 * ($Cn01/$Cn03/$Cn05 = $20/$00/$03: Disk II, ProDOS block devices), select
 * DCSEL 63 and check the identity bytes $09 = 'V', $0A = major >= 47.  A
 * write/read-back of ADDR_L is not enough: other cards and the floating bus
 * can echo it.  CTRL and ADDR are left at 0.  Returns the slot, 0 if none. */
unsigned char vera_detect(void);

/* Wait for the start of the next VERA frame (polls ISR VSYNC, no IRQ needed). */
void vera_wait_vsync(void);

#endif /* VERA_H */
