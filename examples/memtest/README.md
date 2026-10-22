# memtest -- IIe aux RAM + VERA registers + VRAM

The Apple II counterpart of `VERA_C64/tests/test_mem.c` (GeoRAM / RAMbo next to the
VERA there).  Booted straight from the floppy (no DOS), the card found in any slot:

1. **aux RAM $0200-$BFFF** (190 pages): each page is filled in main RAM, moved to
   aux with the IIe's AUXMOVE (`$C311`), moved back and compared; after every page
   the VERA ADDR0_L/ADDR0_M are written and read back.  $0000-$01FF and the aux
   language card are not tested (they switch together with the stack).  Skipped on
   a ][+ or on a IIe without the extended 80-column card.
2. **VRAM $00000-$1F9BF** (below the PSG, palette and sprite registers) through
   DATA0, increment 1: filled, then verified.

Every byte tested also goes straight into the VERA border colour; the border covers
the whole screen (`VSTART = VSTOP = 0`).  At the end the border shows the result:
green = all OK, red = error; the Apple screen lists AUX RAM / VERA REGS / VERA VRAM.

## Build and run

    make
    A2_MODEL=iiee A2_AUX=64 a2emu memtest.dsk
    make test A2RUN="../../a2run --rom ../../rom"   # headless (IIe + 64 KB aux), about 1500 frames
