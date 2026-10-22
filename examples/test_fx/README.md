# test_fx -- VERA FX coprocessor registers and behaviour

Port of `VERA_C64/tests/test_fx.c`, output on the Apple screen.  Booted straight
from the floppy (no DOS), the card found in any slot:

- FX_CTRL (DCSEL 2) write/read-back;
- the other FX registers are write-only: their reads return the DCSEL 63
  identity bytes ('V', 48, 0, 1 on 48.0.1);
- multiplier (3 x 4 into VRAM through the cache), transparency (zero bytes
  skipped), manual cache load + write, cache fill + write (4:1 copy);
- benchmarks over 16 KB: fill and copy, byte by byte vs the 32-bit FX cache.

Only FAIL lines are printed (pausing every 20 lines), then PASS/FAIL counts.
CTRL bit 7 is never written (on a real VERA it reconfigures the FPGA).  The
benchmark ticks are VERA frames (59.5 Hz), counted by polling ISR VSYNC every 256
loop iterations, since the Apple has no clock.

## Build and run

    make
    make run
    make test A2RUN="../../a2run --rom ../../rom"   # headless; prints PASS: 36 FAIL: 0
