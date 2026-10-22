/* vera_demo.c -- a 320x240 bitmap in 256 colours on the VERA layer 0.
 *
 * Booted by sdk/boot.s, no DOS: finds the A2VERA card in any slot, sets
 * layer 0 to an 8 bpp bitmap at VRAM $00000 (320 x 240 = 76,800 bytes,
 * through DATA0 with increment 1), shows it scaled 2x, then cycles the
 * border colour on every VERA frame until a key is pressed.  main never
 * returns.
 */

#include <conio.h>
#include "vera.h"

#define W 320u
#define H 240u

int main(void)
{
    unsigned int x;
    unsigned char y, c;

    clrscr();
    if (!vera_detect()) {
        cputs("VERA card not found in slots 1-7\r\n");
        for (;;)
            ;
    }
    cprintf("VERA in slot %u ($%04X)\r\n", vera_slot, (unsigned)vera);
    cputs("320x240 8bpp bitmap on layer 0\r\n");

    VERA_CTRL        = 0;               /* DCSEL 0 */
    VERA_DC_VIDEO    = 0x01;            /* VGA, all layers off while drawing */
    VERA_DC_HSCALE   = 0x40;            /* 2x: 320 x 240 */
    VERA_DC_VSCALE   = 0x40;
    VERA_DC_BORDER   = 0;
    VERA_L0_CONFIG   = 0x07;            /* bitmap, 8 bpp */
    VERA_L0_TILEBASE = 0x00;            /* bitmap at $00000, 320 wide */
    VERA_L0_HSCR_H   = 0;               /* palette offset 0 */
    VERA_CTRL        = 0x02;            /* DCSEL 1: a 32-pixel border all around */
    VERA_DC_HSTART   = 32 / 4;
    VERA_DC_HSTOP    = (640 - 32) / 4;
    VERA_DC_VSTART   = 32 / 2;
    VERA_DC_VSTOP    = (480 - 32) / 2;
    VERA_CTRL        = 0;

    cputs("Drawing");
    VERA_SET_ADDR(0x00000ul, VERA_INC1);
    for (y = 0; y < H; ++y) {
        /* diagonal bands, a frame of colour 1 around the picture */
        for (x = 0; x < W; ++x) {
            c = (unsigned char)(x + y);
            if (y == 0 || y == H - 1 || x == 0 || x == W - 1)
                c = 1;
            vera_w0(c);
        }
        if ((y & 15) == 0)
            cputc('.');
    }
    VERA_DC_VIDEO = 0x11;               /* VGA + layer 0 */
    cputs(" done\r\n");
    cputs("The border colour changes every frame.\r\n");
    cputs("Press a key to stop.\r\n");

    for (c = 0; !kbhit(); ++c) {
        vera_wait_vsync();
        VERA_CTRL      = 0;
        VERA_DC_BORDER = c;
    }
    cgetc();
    cputs("Test ended.\r\n");
    for (;;)
        ;
}
