/* test_scrolling.c — VERA colour gradient + VSync-locked scroll animation
 * (port of VERA_C64/tests/test_gradient_scroll.c).
 *
 * Fills the full 128×64 VERA tilemap with a diagonal colour gradient, then
 * scrolls layer 1 by 50 pixels in each direction (up / down / right / left).
 * Speed follows a triangle-wave profile: accelerates from MIN_SPEED to
 * MAX_SPEED pixels/frame over successive cycles, then decelerates back,
 * repeating until a key is pressed.
 *
 * Booted by sdk/boot.s, no DOS: the A2VERA card is found in any slot, the
 * VERA text layer is set up by vtext.c (mode chosen at build time), the
 * Apple screen shows only messages, frames are timed by the VERA's own VSYNC
 * flag.  main never returns.
 */

#include <conio.h>
#include "vera.h"
#include "vtext.h"

/* ------------------------------------------------------------------ */
/* VRAM / tilemap constants                                            */
/* ------------------------------------------------------------------ */

/* Tilemap at $01B000; stride = 128 cells * 2 bytes = 256 bytes/row   */
#define MAP_COLS   128
#define MAP_ROWS    64

/* ADDR_H: bank=1 (bit 0), auto-increment by 1 (bits[7:4] = 0x1)     */
#define ADDR_H_INC1  0x11

/* HSCROLL / VSCROLL wrap modulo (12-bit registers; tilemap pixel     */
/* extent = 128*8 = 1024 wide, 64*16 = 1024 tall for 8×16 font; the   */
/* VERA wraps the 512 pixels of the 8×8 font by itself)              */
#define SCROLL_WRAP  1024u

/* Pixels to cover per direction per cycle                             */
#define SCROLL_STEP   50

/* Speed range in pixels/frame — triangle wave between these limits   */
#define MIN_SPEED     1
#define MAX_SPEED     8

/* ------------------------------------------------------------------ */

/* Fill the entire 128×64 tilemap with a diagonal colour gradient.
 * Each cell: char = 0x20 (space), colour byte = (idx<<4)|idx so that
 * bg == fg == gradient index 0–15.                                   */
static void fill_gradient(void)
{
    unsigned char row, col, colour;

    for (row = 0; row < MAP_ROWS; row++)
    {
        /* Row base address: $01B000 + row * 256.
         * ADDR_L is always 0 because stride == 256 and base is
         * aligned to 256.  ADDR_M = $B0 + row (0..63 → $B0..$EF). */
        VERA_CTRL = 0x00;
        VERA_ADDR_L   = 0x00;
        VERA_ADDR_M   = (unsigned char)(0xB0 + row);
        VERA_ADDR_H   = ADDR_H_INC1;

        for (col = 0; col < MAP_COLS; col++)
        {
            colour = (row + col) & 0x0F;         /* 0–15 diagonal     */
            VERA_DATA0 = 0x20;                   /* space character   */
            VERA_DATA0 = (colour << 4) | colour; /* bg == fg          */
        }
    }

}

/* Write HSCROLL and VSCROLL registers for layer 1 (12-bit each).    */
static void set_scroll(unsigned int h, unsigned int v)
{
    VERA_CTRL  = 0x00;
    VERA_L1_HSCR_L = (unsigned char)(h & 0xFF);
    VERA_L1_HSCR_H = (unsigned char)((h >> 8) & 0x0F);
    VERA_L1_VSCR_L = (unsigned char)(v & 0xFF);
    VERA_L1_VSCR_H = (unsigned char)((v >> 8) & 0x0F);
}

/* Scroll vscroll forward by `speed` pixels (with wrap).              */
static unsigned int vscroll_add(unsigned int v, unsigned char speed)
{
    return (v + speed) % SCROLL_WRAP;
}

/* Scroll vscroll backward by `speed` pixels (with wrap).             */
static unsigned int vscroll_sub(unsigned int v, unsigned char speed)
{
    return (v >= speed) ? v - speed : SCROLL_WRAP + v - speed;
}

/* Scroll hscroll forward by `speed` pixels (with wrap).              */
static unsigned int hscroll_add(unsigned int h, unsigned char speed)
{
    return (h + speed) % SCROLL_WRAP;
}

/* Scroll hscroll backward by `speed` pixels (with wrap).             */
static unsigned int hscroll_sub(unsigned int h, unsigned char speed)
{
    return (h >= speed) ? h - speed : SCROLL_WRAP + h - speed;
}

int main(void)
{
    unsigned int hscroll = 0, vscroll = 0;
    unsigned char speed = MIN_SPEED;
    signed char  delta  = 1;           /* +1 = accelerating, -1 = decelerating */
    unsigned char pixels;

    clrscr();
    if (!vera_detect()) {
        cputs("VERA card not found in slots 1-7\r\n");
        for (;;)
            ;
    }
    cprintf("VERA in slot %u ($%04X)\r\n", vera_slot, (unsigned)vera);
    cputs("Gradient and scroll on the VERA\r\n");
    cprintf("(%ux%u). Press a key to stop.\r\n", VT_COLS, VT_ROWS);
    vt_init();
    fill_gradient();

    while (!kbhit())
    {
        /* Scroll UP: VSCROLL increases → viewport moves down → content shifts up */
        for (pixels = 0; pixels < SCROLL_STEP && !kbhit(); pixels += speed)
        {
            vera_wait_vsync();
            vscroll = vscroll_add(vscroll, speed);
            set_scroll(hscroll, vscroll);
        }
        if (kbhit()) break;

        /* Scroll DOWN: VSCROLL decreases → viewport moves up → content shifts down */
        for (pixels = 0; pixels < SCROLL_STEP && !kbhit(); pixels += speed)
        {
            vera_wait_vsync();
            vscroll = vscroll_sub(vscroll, speed);
            set_scroll(hscroll, vscroll);
        }
        if (kbhit()) break;

        /* Scroll RIGHT: HSCROLL increases → viewport moves right → content shifts right */
        for (pixels = 0; pixels < SCROLL_STEP && !kbhit(); pixels += speed)
        {
            vera_wait_vsync();
            hscroll = hscroll_add(hscroll, speed);
            set_scroll(hscroll, vscroll);
        }
        if (kbhit()) break;

        /* Scroll LEFT: HSCROLL decreases → viewport moves left → content shifts left */
        for (pixels = 0; pixels < SCROLL_STEP && !kbhit(); pixels += speed)
        {
            vera_wait_vsync();
            hscroll = hscroll_sub(hscroll, speed);
            set_scroll(hscroll, vscroll);
        }

        /* Triangle-wave speed update: accelerate to MAX then decelerate to MIN */
        speed += delta;
        if (speed >= MAX_SPEED)
        {
            speed = MAX_SPEED;
            delta = -1;
        }
        else if (speed <= MIN_SPEED)
        {
            speed = MIN_SPEED;
            delta = 1;
        }
    }

    cgetc();             /* consume the keypress */
    set_scroll(0, 0);    /* restore scroll origin */
    vt_clear();
    vt_puts("Test ended.\n");
    cputs("Test ended.\r\n");
    for (;;)
        ;
}
