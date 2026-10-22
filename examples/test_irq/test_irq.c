/* test_irq.c — VERA IRQ hook test (port of VERA_C64/tests/test_irq.c).
 *
 * Booted by sdk/boot.s, no DOS: finds the A2VERA card in any slot; output
 * goes to the Apple screen.  Exercises ../vera/vera_irq.s:
 *
 *  1. hook installed on the 6502 IRQ vector ($FFFE in LC RAM), the previous
 *     one kept for chaining
 *  2. VSYNC IRQ rate ~ 59.52 Hz (25 MHz / 800 / 525)
 *  3. LINE IRQ at raster line 240, once per VERA frame
 *  4. AUDIO FIFO LOW is masked after the first IRQ (cannot be acknowledged)
 *     and the machine keeps running (no IRQ storm)
 *  5. VRAM accesses of the main program are not disturbed by the IRQs (the
 *     hook never touches ADDR, DATA, CTRL)
 *  6. vera_irq_remove() disables the sources and restores the vector
 *
 * The Apple has no clock: the time base is delay_ms(), counted in CPU cycles
 * (the time in the IRQ handler, ~0.5 %, comes on top).  main never returns.
 */

#include <conio.h>
#include "vera.h"
#include "vera_irq.h"

void __fastcall__ delay_ms(unsigned int ms);

#define MEASURE_MS 2000u   /* ~119 VERA frames */

static unsigned char g_pass = 0;
static unsigned char g_fail = 0;
static unsigned char g_lines = 0;
static const char *g_section = "";

static void pause_page(void)
{
    if (g_lines >= 20)
    {
        cputs("-- press a key --\r\n");
        cgetc();
        g_lines = 0;
    }
}

static void check(const char *name, unsigned char ok, unsigned int got, unsigned int exp)
{
    if (ok)
    {
        g_pass++;
        return;
    }
    pause_page();
    cprintf("FAIL %s %s\r\n     got %u exp %u\r\n", g_section, name, got, exp);
    g_lines += 2;
    g_fail++;
}

/* Expected VERA events in 'ms' milliseconds: 59.5238 Hz */
static unsigned int expected_events(unsigned int ms)
{
    return (unsigned int)(((unsigned long)ms * 595238ul) / 10000000ul);
}

static unsigned char near_enough(unsigned int got, unsigned int exp)
{
    unsigned int tol = exp / 20u + 1u;   /* 5 % + 1 */
    return got + tol >= exp && got <= exp + tol;
}

int main(void)
{
    unsigned char before, flags, i, bad;
    unsigned int exp, got, n;
    unsigned int old_vec;

    clrscr();
    if (!vera_detect()) {
        cputs("VERA card not found in slots 1-7\r\n");
        for (;;)
            ;
    }

    cprintf("VERA IRQ hook test (slot %u)\r\n", vera_slot);
    cputs("==================\r\n");

    old_vec = vera_irq_vector();

    /* Start from a known VERA state: all IRQs off, status clear */
    VERA_IEN = 0;
    VERA_ISR = 0x07;

    /* [1] install */
    g_section = "[1]";
    vera_irq_install();
    check("vector changed", vera_irq_vector() != old_vec, vera_irq_vector(), old_vec);
    check("no IRQ enabled yet", (VERA_IEN & 0x0F) == 0, VERA_IEN & 0x0F, 0);

    /* [2] VSYNC */
    g_section = "[2] VSYNC";
    vera_irq_take();
    before = vera_irq_count[0];
    vera_irq_enable(VERA_IRQ_VSYNC);
    delay_ms(MEASURE_MS);
    vera_irq_disable(VERA_IRQ_VSYNC);
    got = (unsigned char)(vera_irq_count[0] - before);
    exp = expected_events(MEASURE_MS);
    check("count ~ 59.52 Hz", near_enough(got, exp), got, exp);
    flags = vera_irq_take();
    check("flag VSYNC set", (flags & VERA_IRQ_VSYNC) != 0, flags, VERA_IRQ_VSYNC);
    check("IEN VSYNC off after disable", (VERA_IEN & VERA_IRQ_VSYNC) == 0, VERA_IEN & 1, 0);

    /* [3] LINE */
    g_section = "[3] LINE";
    vera_irq_set_line(240);
    vera_irq_take();
    before = vera_irq_count[1];
    vera_irq_enable(VERA_IRQ_LINE);
    delay_ms(MEASURE_MS);
    vera_irq_disable(VERA_IRQ_LINE);
    got = (unsigned char)(vera_irq_count[1] - before);
    exp = expected_events(MEASURE_MS);
    check("count ~ one per VERA frame", near_enough(got, exp), got, exp);
    flags = vera_irq_take();
    check("flag LINE set", (flags & VERA_IRQ_LINE) != 0, flags, VERA_IRQ_LINE);

    /* [4] AFLOW: FIFO is empty after reset, so the IRQ is pending at once */
    g_section = "[4] AFLOW";
    before = vera_irq_count[3];
    vera_irq_enable(VERA_IRQ_AFLOW);
    delay_ms(100);
    got = (unsigned char)(vera_irq_count[3] - before);
    check("serviced exactly once (then masked)", got == 1, got, 1);
    check("IEN AFLOW masked by the hook", (VERA_IEN & VERA_IRQ_AFLOW) == 0, VERA_IEN & 8, 0);
    flags = vera_irq_take();
    check("flag AFLOW set", (flags & VERA_IRQ_AFLOW) != 0, flags, VERA_IRQ_AFLOW);

    /* [5] VRAM traffic under VSYNC IRQs: write, read back, ~0.5 s */
    g_section = "[5] VRAM";
    before = vera_irq_count[0];
    vera_irq_enable(VERA_IRQ_VSYNC);
    bad = 0;
    for (n = 0; n < 40; ++n) {
        VERA_SET_ADDR(0x04000ul, VERA_INC1);
        i = 0;
        do { vera_w0((unsigned char)(i ^ n)); } while (++i);
        VERA_SET_ADDR(0x04000ul, VERA_INC1);
        i = 0;
        do { if (VERA_DATA0 != (unsigned char)(i ^ n)) ++bad; } while (++i);
    }
    vera_irq_disable(VERA_IRQ_VSYNC);
    got = (unsigned char)(vera_irq_count[0] - before);
    check("IRQs taken meanwhile", got != 0, got, 1);
    check("VRAM intact", bad == 0, bad, 0);
    vera_irq_take();

    /* [6] remove */
    g_section = "[6] remove";
    vera_irq_remove();
    check("vector restored", vera_irq_vector() == old_vec, vera_irq_vector(), old_vec);
    check("all sources disabled", (VERA_IEN & 0x0F) == 0, VERA_IEN & 0x0F, 0);

    cputs("\r\n==================\r\n");
    cprintf("PASS: %d   FAIL: %d\r\n", (unsigned int)g_pass, (unsigned int)g_fail);
    for (;;)
        ;
}
