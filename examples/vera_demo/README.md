# vera_demo -- a 320x240 bitmap in 256 colours

Booted straight from the floppy (no DOS), it finds the A2VERA card in any slot and:

- sets layer 0 to an 8 bpp bitmap at VRAM $00000 (`L0_CONFIG = $07`,
  `L0_TILEBASE = $00`), scaled 2x (`HSCALE = VSCALE = $40`);
- draws diagonal colour bands with a frame of colour 1 (76,800 bytes through
  DATA0, increment 1) with the layer off, then enables it (`DC_VIDEO = $11`);
- opens a 32-pixel border (DCSEL 1: HSTART/HSTOP/VSTART/VSTOP) and changes its
  colour on every VERA frame.

## Build and run

    make
    make run             # boot it in a2emu
    make test A2RUN="../../a2run --rom ../../rom"   # headless, VERA shot in vera_demo.png
