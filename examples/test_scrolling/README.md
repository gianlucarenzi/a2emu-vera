# test_scrolling -- VERA colour gradient + VSYNC-locked scroll

Port of `VERA_C64/tests/test_gradient_scroll.c`.  Booted straight from the floppy
(no DOS), it finds the A2VERA card in any slot, fills the 128x64 tilemap of layer 1
with a diagonal colour gradient and scrolls it 50 pixels up, down, right and left,
the speed going 1 -> 8 -> 1 pixels per frame, until a key is pressed.

Frames are timed by the VERA itself: `vera_wait_vsync()` clears ISR VSYNC and polls
it (no IRQ needed; the Apple II has no frame clock of its own).

## Build and run

    make                 # test_scrolling.dsk (VT_MODE=8; 4 and 6 as in test_font)
    make run             # boot it in a2emu
    make test A2RUN="../../a2run --rom ../../rom"   # headless, VERA shot in test_scrolling.png
