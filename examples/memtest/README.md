# VERA Memory Test for Apple II

A simple test program that verifies VERA's 128 KB VRAM through the memory window.

## Files

- `memtest.c`: Test program that writes and verifies a 64 KB pattern in VERA VRAM
- `Makefile`: Build rules for the test

## Building

```bash
make           # Build memtest.dsk
make test      # Run headless in a2run (Apple II screen only)
make run       # Boot in a2emu (shows both Apple II and VERA screens)
make clean     # Remove build artifacts
```

## Test Details

The test:
1. Fills 64 KB of VERA VRAM with a pattern: `byte = (address ^ (address >> 8))`
2. Reads it back and verifies each byte
3. Shows results on screen
4. Sets border color: Green (color 5) if OK, Red (color 2) if errors

## Models

- Default: Enhanced Apple IIe (`iiee`)
- Apple II+ (`MODEL=ii+`): `make MODEL=ii+ run`

## Output

When run with `a2emu`:
- **Apple II window**: Shows test progress and results
- **VERA window**: Shows real-time VRAM activity during test
