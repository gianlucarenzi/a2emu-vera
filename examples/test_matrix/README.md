# test_matrix -- digital rain on the VERA

Port of `VERA_C64/tests/test_matrix.c`.  Booted straight from the floppy (no DOS),
it finds the A2VERA card in any slot, sets up the VERA text console
(`../vera/vtext.c`), loads its own five palette entries (black, white, light and
dark green, black cell background) and lets random "drops" fall down the columns,
two rows per VERA frame, until a key is pressed; then it restores the default
palette entries and the border.  The LFSR seed is the VERA's scanline.

## Build and run

    make                 # test_matrix.dsk (VT_MODE=8; 4 and 6 as in test_font)
    make run             # boot it in a2emu
    make test A2RUN="../../a2run --rom ../../rom"   # headless, VERA shot in test_matrix.png
