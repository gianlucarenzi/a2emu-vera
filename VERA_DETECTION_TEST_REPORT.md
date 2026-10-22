# VERA slot detection -- test report

`examples/vera/vera.c` finds the A2VERA card at run time, so the same program works
in any slot (`A2_VERA_SLOT=<1-7>` for a2emu, `--vera-slot N` for a2run).

## Method

For slots 1-7:

1. Only reads first: a slot whose `$Cn01/$Cn03/$Cn05` read `$20/$00/$03` holds
   disk-controller firmware (Disk II, ProDOS block devices) and is skipped, so
   nothing is written to it.
2. `CTRL = $7E` (DCSEL 63), then the identity bytes: `$Cn09 = 'V'` and
   `$Cn0A` (major version) `>= 47`; `CTRL = 0` again.
3. On a match `vera` points at `$Cn00`, `vera_slot = n`, ADDR is cleared.

A write/read-back of ADDR_L (the first version) is not a valid probe: other cards
and the floating bus can echo it, and it writes into every slot.

## Emulator side

The Disk II firmware always occupies slot 6, the hard disk's slot 7 once an image
is inserted; their firmware wins the reads of `$Cnxx`.  a2emu/a2run warn:

    a2emu: warning: slot 6 holds the Disk II, the VERA card there is unreachable
    a2emu: warning: the hard disk takes slot 7, the VERA card there is unreachable

## Results

`examples/test_font/test_font.dsk` (first line of the Apple screen after 200
frames), a2run, 2026-10-22:

| VERA slot | ][+ | IIe | enhanced IIe |
|-----------|-----|-----|--------------|
| 1 | found, slot 1 | found, slot 1 | found, slot 1 |
| 2 | found, slot 2 | found, slot 2 | found, slot 2 |
| 3 | found, slot 3 | found, slot 3 | found, slot 3 |
| 4 (default) | found, slot 4 | found, slot 4 | found, slot 4 |
| 5 | found, slot 5 | found, slot 5 | found, slot 5 |
| 6 | not found (Disk II), warning | not found, warning | not found, warning |
| 7 | found, slot 7 | found, slot 7 | found, slot 7 |
| none (`--vera-slot 0`) | not found | not found | not found |

With a hard disk image (`--hd`): VERA in slot 4 found; VERA in slot 7 not found,
with the warning.

The identity bytes read `'V'`, 48, 0, 1 (VERA 48.0.1).

## Reproduce

    cd examples/test_font && make
    for s in 1 2 3 4 5 6 7 0; do
      ../../a2run --rom ../../rom --model iiee --vera-slot $s --disk1 test_font.dsk \
        --no-writeback --frames 200 --text --quiet | head -1
    done
