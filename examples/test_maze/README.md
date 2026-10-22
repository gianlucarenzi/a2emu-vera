# test_maze -- the "10 PRINT" maze on the VERA

Port of `VERA_C64/tests/test_maze.c`.  Booted straight from the floppy (no DOS),
it finds the A2VERA card in any slot, sets up the VERA text console
(`../vera/vtext.c`) and prints the two diagonal glyphs (6 and 7) at random -- a
page from the cc65 `rand()` (bit 8), the next from an 8-bit LFSR -- until a key is
pressed.  The LFSR seed is the VERA's scanline: the Apple has no timer of its own.

## Build and run

    make                 # test_maze.dsk (VT_MODE=8; 4 and 6 as in test_font)
    make run             # boot it in a2emu
    make test A2RUN="../../a2run --rom ../../rom"   # headless, VERA shot in test_maze.png
