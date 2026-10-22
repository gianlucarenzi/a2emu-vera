/* test_font.c - every glyph of the VERA text font, over and over
 * (port of VERA_C64/tests/test_font.c).
 *
 * Booted by sdk/boot.s, no DOS: finds the A2VERA card in any slot, sets up a
 * text console on the VERA (vtext.c, mode chosen at build time with VT_MODE)
 * and prints the 256 tile codes (128 glyphs, then the same in inverse video)
 * until a key is pressed.  main never returns.
 */

#include <conio.h>
#include "vera.h"
#include "vtext.h"

int main(void)
{
    unsigned int i;

    clrscr();
    if (!vera_detect()) {
        cputs("VERA card not found in slots 1-7\r\n");
        for (;;)
            ;
    }
    VERA_CTRL = 0x7E;                   /* DCSEL 63: the version */
    cprintf("VERA %u.%u.%u in slot %u ($%04X)\r\n",
            VERA_REG0A, VERA_REG0B, VERA_REG0C, vera_slot, (unsigned)vera);
    VERA_CTRL = 0;
    cputs("Character test loop on the VERA\r\n");
    cprintf("(%ux%u). Press a key to stop.\r\n", VT_COLS, VT_ROWS);

    vt_init();
    vt_puts("VERA detected, character test loop. Press a key to stop.\n");
    while (1) {
        for (i = 0; i < 256; i++) {
            vt_glyph((unsigned char)i);
            if (kbhit())
                goto done;
        }
    }
done:
    cgetc();
    vt_puts("\nTest ended.\n");
    cputs("Test ended.\r\n");
    for (;;)
        ;
}
