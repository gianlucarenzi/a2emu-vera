# VERA Font/Graphics Display Test for Apple II

This is a port of the VERA font test from the C64 test suite. It fills the VERA framebuffer with various graphical patterns to test color rendering capabilities.

## What It Does

The test fills the VERA 64KB framebuffer with three different patterns:

1. **Vertical Color Stripes** - Changes color every 256 bytes (every scanline)
2. **Checkerboard Pattern** - Alternating black and white pixels
3. **Diagonal Gradient** - Color gradually changes along diagonals

Each test fills the entire framebuffer and waits for you to press RETURN.

## Building

```bash
make
```

This creates `test_font.dsk` which can be run with a2emu.

## Running

```bash
a2emu test_font.dsk
```

Or with a specific VERA slot:

```bash
A2_VERA_SLOT=5 a2emu test_font.dsk
```

## What to Expect

If VERA rendering is working:
- You should see colored patterns on the VERA window
- The patterns should correspond to what's being written to VRAM
- Each pattern should be visually distinct

## Test Output

The Apple II screen shows:
- Detection of VERA card and which slot it's in
- Progress bars (dots) while filling the framebuffer
- Prompts to press RETURN between tests

The VERA window should show the actual rendered patterns.

## Notes

This is a simplified version compared to the C64 test_font.c:
- The C64 version used a text console (vtext.c) to display 256 glyphs
- The Apple II version fills the framebuffer with graphics patterns instead
- Both test the VERA rendering capabilities

The test uses the vera_detect.h library for runtime slot detection, so it works regardless of which slot the emulator puts VERA in.

## Porting From C64

Original source: `~/Progetti-CVS/C64/VERA_C64/tests/test_font.c`

Key differences:
- C64 version: text mode, 256 glyphs (including inverse video)
- Apple II version: framebuffer mode, graphics patterns
- Both detect VERA at runtime
- Both test rendering capabilities
