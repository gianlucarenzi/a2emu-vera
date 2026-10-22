#ifndef VERA_IRQ_H
#define VERA_IRQ_H

/*
 * VERA IRQ hook on the Apple II (vera_irq.s; port of VERA_C64/tests/vera_irq.h).
 *
 * The A2VERA's /IRQ drives the slot's /IRQ line (a2emu: A2_VERA_IRQ, default
 * on).  A cc65 program booted by sdk/boot.s runs with the language card RAM
 * read enabled, so the hook goes into the 6502 vector at $FFFE in LC RAM; an
 * IRQ that is not the VERA's goes on to the previous vector, if it had one.
 *
 * IRQ_n is level sensitive; for every serviced source the hook:
 *   VSYNC, LINE, SPRCOL : acknowledges it (write-1-to-clear in ISR)
 *   AUDIO FIFO LOW      : cannot be acknowledged, so it is masked in IEN
 *                         (refill the FIFO, then vera_irq_enable() again)
 * and records it in the flags returned by vera_irq_take().
 *
 * The hook touches only IEN, ISR and IRQ_LINE_L: ADDR, DATA, CTRL
 * (ADDRSEL/DCSEL) are never touched, so it can interrupt code that is in the
 * middle of a VRAM access sequence.
 */

#define VERA_IRQ_VSYNC   0x01
#define VERA_IRQ_LINE    0x02
#define VERA_IRQ_SPRCOL  0x04
#define VERA_IRQ_AFLOW   0x08

/* Install the hook (idempotent), after vera_detect(); enables 6502 IRQs (CLI).
 * Does not enable any VERA IRQ. */
void vera_irq_install(void);

/* Disable all VERA IRQs and restore the vector (if nobody hooked it after us). */
void vera_irq_remove(void);

/* Enable / disable the VERA IRQ sources in 'mask' (VERA_IRQ_*). Stale status
 * of VSYNC/LINE/SPRCOL is cleared before enabling. */
void __fastcall__ vera_irq_enable(unsigned char mask);
void __fastcall__ vera_irq_disable(unsigned char mask);

/* Return the sources serviced since the last call and clear them (atomic). */
unsigned char vera_irq_take(void);

/* Raster line (0-511) for the LINE IRQ. */
void __fastcall__ vera_irq_set_line(unsigned int line);

/*
 * Optional callback, run in IRQ context after the sources are acknowledged,
 * with A = serviced sources (VERA_IRQ_*).  It must be a plain 6502 routine
 * ending in RTS (A, X and Y may be changed); do not call cc65-compiled C
 * from it.  Pass 0 to remove.
 */
void __fastcall__ vera_irq_set_callback(void (*callback)(void));

/* The IRQ vector at $FFFE as the CPU reads it now. */
unsigned int vera_irq_vector(void);

/* Serviced-event counters, one per source (VSYNC, LINE, SPRCOL, AFLOW), 8-bit wrap. */
extern volatile unsigned char vera_irq_count[4];

#endif /* VERA_IRQ_H */
