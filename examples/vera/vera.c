/* vera.c -- find the A2VERA card's slot (see vera.h). */

#include "vera.h"

volatile unsigned char *vera = (volatile unsigned char *)0xC400;
unsigned char vera_slot = 0;

unsigned char vera_detect(void)
{
    unsigned char s, v, m;
    volatile unsigned char *p;

    for (s = 1; s <= 7; ++s) {
        p = (volatile unsigned char *)(0xC000u + ((unsigned)s << 8));
        /* only reads here: a disk controller's firmware is left alone */
        if (p[1] == 0x20 && p[3] == 0x00 && p[5] == 0x03)
            continue;
        p[5] = 0x7E;                    /* CTRL: DCSEL 63 */
        v = p[9];
        m = p[10];
        p[5] = 0x00;
        if (v == 'V' && m >= 47) {
            vera = p;
            vera_slot = s;
            VERA_ADDR_L = 0;
            VERA_ADDR_M = 0;
            VERA_ADDR_H = 0;
            return s;
        }
    }
    return 0;
}

void vera_wait_vsync(void)
{
    VERA_ISR = VERA_ISR_VSYNC;          /* clear a VSYNC already pending */
    while (!(VERA_ISR & VERA_ISR_VSYNC))
        ;
}
