/* test_maze.c - the "10 PRINT" maze with the two diagonal glyphs
 * (port of VERA_C64/tests/test_maze.c).
 *
 * Booted by sdk/boot.s, no DOS: finds the A2VERA card in any slot, sets up
 * its own text console on the VERA (vtext.c, mode chosen at build time) and
 * prints glyph 6 or 7 at random, one page with the cc65 rand(), the next one
 * with an 8 bit LFSR, until a key is pressed.  main never returns.
 */

#include <stdlib.h>
#include <conio.h>
#include "vera.h"
#include "vtext.h"

/* 8-bit Galois LFSR, x^8+x^6+x^5+x^2+1 (as in test_matrix.c) */
static unsigned char prng8;

static unsigned char fast_rand(void)
{
    unsigned char lsb = prng8 & 1;
    prng8 >>= 1;
    if (lsb) prng8 ^= 0xB4;
    return prng8;
}

int main(void)
{
    int maze;
    int do_rand = 1;
    int counter = VT_COLS * 25;

    clrscr();
    if (!vera_detect()) {
        cputs("VERA card not found in slots 1-7\r\n");
        for (;;)
            ;
    }
    cprintf("VERA in slot %u ($%04X)\r\n", vera_slot, (unsigned)vera);
    cputs("Maze test loop on the VERA\r\n");
    cprintf("(%ux%u). Press a key to stop.\r\n", VT_COLS, VT_ROWS);
    srand(12345);

    /* seed: the VERA's scanline (the Apple has no timer of its own) */
    prng8 = VERA_IRQLINE_L;
    if (!prng8) prng8 = 0xA5;   /* LFSR must not be zero */

    vt_init();

    while (1)
    {
        if (kbhit())
            break;

        if (--counter > 1)
        {
            if (do_rand)
            {
                /* first a page of random values from the C rand(); its
                 * low bit only alternates (diagonal stripes): take bit 8 */
                maze = (rand() >> 8) & 0x01;
            }
            else
            {
                /* then the random numbers of the LFSR */
                maze = fast_rand() & 0x01;
            }
            /* print the maze */
            vt_glyph((unsigned char)(maze + 6));
        }
        else
        {
            counter = VT_COLS * 25;
            do_rand = !do_rand;
        }
    }
    cgetc();
    vt_puts("\nTest ended.\n");
    cputs("Test ended.\r\n");
    for (;;)
        ;
}
