# VERA Scrolling Test for Apple II

This is a port of the VERA gradient and scrolling test from the C64 test suite.

## What It Does

The test:
1. Fills the VERA framebuffer with a diagonal gradient pattern
2. Tests horizontal scrolling (HSCROLL register)
3. Tests vertical scrolling (VSCROLL register)
4. Demonstrates all four scroll directions

## Building

```bash
make
```

This creates `test_scrolling.dsk` which can be run with a2emu.

## Running

```bash
a2emu test_scrolling.dsk
```

Or with a specific VERA slot:

```bash
A2_VERA_SLOT=6 a2emu test_scrolling.dsk
```

## What to Expect

- The Apple II screen shows test progress
- The VERA window should display a diagonal gradient pattern
- As the test runs, the pattern should scroll (right, down, left, up)
- The pattern should wrap around smoothly
- At the end, the pattern returns to the starting position

## Technical Details

The test uses VERA's Display Controller (DC) registers:
- **DCSEL=3**: Layer scroll registers
- **DC_L0_HSCROLL_L/H**: Horizontal scroll (12-bit, DCSEL=3, offsets 0x09/0x0A)
- **DC_L0_VSCROLL_L/H**: Vertical scroll (12-bit, DCSEL=3, offsets 0x0B/0x0C)

Scroll values are 12-bit, allowing for smooth continuous scrolling.

## Porting From C64

Original source: `~/Progetti-CVS/C64/VERA_C64/tests/test_gradient_scroll.c`

Key differences:
- C64 version: used text layer with tilemap-based scrolling
- Apple II version: uses framebuffer-based scrolling
- C64 had VSync-locked animation, Apple II version shows direct scroll changes
- Both test the core VERA scroll register functionality

## Notes

This test is essential for verifying:
- VRAM write/read through DATA0
- Display Controller register access via CTRL
- Layer 0 scroll register functionality (HSCROLL, VSCROLL)
- Proper coordinate wrapping in scroll values
