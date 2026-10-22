# dblres -- IIe double hi-res / double lo-res

Colour bars for the IIe's double-res video modes, which work only with aux memory
(the extended 80-column card, a RamWorks or the RAM2E: `--aux 64K` or more).
Raw 6502 code booted straight from the floppy (no DOS); a key steps through:

1. **double hi-res**: 16 bars of 8 colour pixels (32 dots) in palette order,
   black ... white, then black;
2. **double lo-res**: 16 bars of 5 blocks;
3. **DHIRES off** (`$C05F`): the main half of the same memory as plain hi-res.

`gendata.py` writes the bars into `data.inc`: a colour is stored rotated right by one
bit (the display rotates it back, as MAME does), DHGR 4 dots per colour with the first
dot in bit 0 and the bytes alternating aux, main.  Aux is written with 80STORE + PAGE2.

Without aux memory, or on a ][+, step 1 looks exactly like step 3: `$C05E` changes nothing.

## Build and run

    make
    make run                     # enhanced IIe, 64 KB aux
    make run MODEL=iie AUX=RAM2E
