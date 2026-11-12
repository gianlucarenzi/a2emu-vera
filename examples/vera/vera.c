/* vera.c -- find the VERA card's slot: A2VERA or VERA Module RBL-A2 (see vera.h). */

#include "vera.h"

volatile unsigned char *vera = (volatile unsigned char *)0xC400;
unsigned char vera_slot = 0;

unsigned char vera_detect(void)
{
    unsigned char s, v, m;
    volatile unsigned char *p;

    for (s = 1; s <= 7; ++s) {
        p = (volatile unsigned char *)(0xC000u + ((unsigned)s << 8));
        /* only reads here: a disk controller's firmware is left alone ... */
        if (p[1] == 0x20 && p[3] == 0x00 && p[5] == 0x03) {
            /* ... unless it is the VERA Module RBL-A2's: after a reset its ProDOS
               slot ROM ("VERAS" at $CnF0) hides the VERA; $C0nF bit 0 = 1 puts
               the VERA at $Cn00-$Cn1F (the ROM above $Cn1F stays) */
            if (p[0xF0] != 'V' || p[0xF1] != 'E' || p[0xF2] != 'R' ||
                p[0xF3] != 'A' || p[0xF4] != 'S')
                continue;
            *(volatile unsigned char *)(0xC08Fu + ((unsigned)s << 4)) = 0x01;
        }
        /* an empty slot reads the floating bus, which can show anything once:
           CTRL must read back what was written, the identity twice */
        p[5] = 0x7E;                    /* CTRL: DCSEL 63 */
        if (p[5] != 0x7E)
            continue;
        v = p[9];
        m = p[10];
        p[5] = 0x00;
        if (p[5] != 0x00 || v != 'V' || m < 47)
            continue;
        p[5] = 0x7E;
        if (p[9] != v || p[10] != m || p[5] != 0x7E) {
            p[5] = 0x00;
            continue;
        }
        p[5] = 0x00;
        vera = p;
        vera_slot = s;
        vera_io_init((unsigned char)(0xC0 + s));
        VERA_ADDR_L = 0;
        VERA_ADDR_M = 0;
        VERA_ADDR_H = 0;
        return s;
    }
    return 0;
}

void vera_wait_vsync(void)
{
    VERA_ISR = VERA_ISR_VSYNC;          /* clear a VSYNC already pending */
    while (!(VERA_ISR & VERA_ISR_VSYNC))
        ;
}
