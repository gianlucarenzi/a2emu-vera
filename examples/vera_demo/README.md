# VERA Graphics Demo for Apple II

A simple demo program that fills the VERA display with a gradient pattern.

## Files

- `vera_demo.c`: Demo program that initializes VERA in graphics mode and fills VRAM
- `Makefile`: Build rules for the demo

## Building

```bash
make           # Build vera_demo.dsk
make test      # Run headless in a2run (Apple II screen only)
make run       # Boot in a2emu (shows both Apple II and VERA screens)
make clean     # Remove build artifacts
```

## Demo Details

The program:
1. Initializes VERA for 640x480 8-bit graphics mode
2. Sets up a grayscale palette (256 shades)
3. Fills all 64 KB of VERA VRAM with a XOR pattern
4. Shows progress on the Apple II screen
5. Returns when user presses RETURN

## Output

When run with `a2emu`:
- **Apple II window**: Shows initialization progress and control instructions
- **VERA window**: Displays the filled pattern in real-time
