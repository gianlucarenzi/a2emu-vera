# test_font -- every glyph of the VERA text font

Port of `VERA_C64/tests/test_font.c`.  Booted straight from the floppy (sdk/boot.s,
no DOS), it finds the A2VERA card in any slot (`../vera/vera.c`), sets up a text
console on the VERA layer 1 (`../vera/vtext.c`) and prints the 256 tile codes --
128 glyphs, then the same in inverse video -- until a key is pressed.  The Apple
screen shows the VERA version, its slot and the messages.

## Build and run

    make                 # test_font.dsk, 80x30 (8x16 font)
    make VT_MODE=4       # 40x30, 8x8 font scaled 2x
    make VT_MODE=6       # 80x60, 8x8 font
    make run             # boot it in a2emu (MODEL=plus for the ][+)
    make test A2RUN="../../a2run --rom ../../rom"   # headless, VERA shot in test_font.png

Any slot: `A2_VERA_SLOT=5 a2emu test_font.dsk`.

## Font

`../vera/font8x8.bin`, `../vera/font8x16.bin`: 128 glyphs in ASCII order, from
VERA_C64 (`fonts/`, BSD 2-Clause), loaded at VRAM $1F000; tilemap 128x64 at $1B000.
