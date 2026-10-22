# test_irq -- VERA IRQ hook

Port of `VERA_C64/tests/test_irq.c`, output on the Apple screen.  The hook
(`../vera/vera_irq.s`) goes into the 6502 IRQ vector at $FFFE: a cc65 program booted
by sdk/boot.s runs with the language card RAM read enabled, so that is where the CPU
reads it (no DOS, no ROM IRQ path).  Checks:

1. hook installed, previous vector kept; no VERA source enabled yet
2. VSYNC IRQs ~ 59.52 Hz (25 MHz / 800 / 525) over 2 s
3. LINE IRQ at line 240, once per VERA frame
4. AFLOW serviced exactly once, then masked by the hook (no IRQ storm)
5. VRAM writes/reads of the main program intact while VSYNC IRQs come in
   (the hook never touches ADDR, DATA, CTRL)
6. remove: vector restored, all sources off

The Apple has no clock: the time base is `../vera/delay.s` (`delay_ms()`, 1,020
cycles per ms, measured exact in a2run).  Needs the card's IRQ wired
(a2emu `A2_VERA_IRQ=1`, the default; with `--vera-irq off` [2]-[4] fail).

## Build and run

    make
    make run
    make test A2RUN="../../a2run --rom ../../rom"   # PASS: 14 FAIL: 0
