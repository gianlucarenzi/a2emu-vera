/* memtest.c -- Test VERA on Apple II with aux memory.
 * Fill and verify 64 KB of aux RAM via VERA's memory window at $C100-$C1FF.
 * VERA provides a 16-bit address space into its 128 KB VRAM.
 *
 * The test writes patterns, reads them back, and shows results.
 * Green border = OK, Red = ERROR.
 */

#include <stdio.h>
#include <stdlib.h>

/* Apple II memory locations */
#define VERA_ADDR_L     0xC000   /* VERA address low byte */
#define VERA_ADDR_M     0xC001   /* VERA address mid byte */
#define VERA_ADDR_H     0xC002   /* VERA address high byte (usually 0) */
#define VERA_DATA0      0xC003   /* VERA data port */
#define VERA_CTRL       0xC005   /* VERA control register */

#define BORDER_COLOR    0xC034   /* Border color register */

#define OK_COLOR        5        /* Green */
#define ERR_COLOR       2        /* Red */

typedef unsigned char byte;
typedef unsigned int  word;

/* Volatile pointers for memory-mapped I/O */
static volatile byte *vera_addr_l = (volatile byte *)VERA_ADDR_L;
static volatile byte *vera_addr_m = (volatile byte *)VERA_ADDR_M;
static volatile byte *vera_addr_h = (volatile byte *)VERA_ADDR_H;
static volatile byte *vera_data0  = (volatile byte *)VERA_DATA0;
static volatile byte *vera_ctrl   = (volatile byte *)VERA_CTRL;
static volatile byte *border      = (volatile byte *)BORDER_COLOR;

/* Set VERA address to a 17-bit address (VRAM offset) */
static void vera_set_addr(word addr)
{
    *vera_addr_l = (byte)(addr & 0xFF);
    *vera_addr_m = (byte)((addr >> 8) & 0xFF);
    /* Note: addr is only 16-bit, so high byte is always 0 for 64KB test */
    *vera_addr_h = 0;
    *vera_ctrl = 0x00;  /* ADDRSEL=0, auto-increment off */
}

int main(void)
{
    word addr, errors = 0;
    byte val, expected;

    printf("VERA MEMORY TEST\n");
    printf("Testing 64KB of VERA VRAM\n\n");

    /* Test 1: Fill with pattern */
    printf("Writing pattern...");
    vera_set_addr(0);
    for (addr = 0; addr < 0x10000; addr++) {
        val = (byte)((addr ^ (addr >> 8)) & 0xFF);
        *vera_data0 = val;
        if ((addr & 0x0FFF) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf(" OK\n");

    /* Test 2: Verify pattern */
    printf("Verifying pattern...");
    errors = 0;
    vera_set_addr(0);
    for (addr = 0; addr < 0x10000; addr++) {
        val = *vera_data0;
        expected = (byte)((addr ^ (addr >> 8)) & 0xFF);
        if (val != expected) {
            if (errors++ < 10) {
                printf("\nError at $%04X: got %02X, expected %02X\n", addr, val, expected);
            }
        }
        if ((addr & 0x0FFF) == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf(" %s\n", errors ? "FAILED" : "OK");

    /* Show result */
    if (errors) {
        printf("ERRORS: %u bytes mismatched\n", errors);
        *border = ERR_COLOR;  /* Red border */
    } else {
        printf("ALL TESTS PASSED\n");
        *border = OK_COLOR;   /* Green border */
    }

    printf("Press RETURN to exit\n");
    getchar();

    return errors ? 1 : 0;
}
