# ramworks -- IIe aux memory banking (RamWorks III / RAM2E II)

Two raw 6502 tests: `ramworks.s` (the banking, every `--aux` size) and `ram2e.s`
(the RAM2E II's configuration commands).

## ramworks.s

Checks the RamWorks-style aux card that `--aux 512K`..`8M` fits (`--aux 64K` = the
plain extended 80-column card, one bank).  Raw 6502 code at $0400, run with 80STORE on
and PAGE2 off so it stays in main RAM whatever RAMRD selects:

1. main markers ($A5) at $3000, $F0 and $D000 (language card bank 2);
2. for bank 127 down to 0: `STA $C073`, then the bank number goes to aux $3000 (and its
   complement to $3001) with RAMWRT, and to aux $F0 and aux $D000 with ALTZP;
3. for bank 0 up: select it, read the values back with RAMRD and ALTZP, stop at the
   first mismatch (bank numbers past the banks fitted mirror the lower ones);
4. bank 0 again, main markers checked.

Exit code (debug card, `$C0B1`): the number of banks that kept their own data, 200 if
main RAM was overwritten (expected without aux memory: ALTZP then maps nothing).

## ram2e.s

`--aux RAM2E`, code at $0800, routines as in Garrett's Workshop's `GWRAM.SYSTEM`
(`ram2e_hal.c`).  Three runs, the phase byte at $0300:

- **0**, fresh card: MAX chip detect ($FF), not a MachXO2 ($FD), LED present ($F0),
  rev. C ($F1), default 128 banks, LED off; LED on ($E2) read back ($E3); capacity
  mask $07 / $00 / $FF = 8 / 1 / 128 banks ($E0); a command with 7 idle cycles
  between two writes still works, with 9 it times out; then mask $0F + LED on are
  saved (UFM bitbang $EA, program $EF);
- **1**, next power-on with that settings file: 16 banks, LED on; erase ($EE);
- **2**, next power-on: 128 banks, LED off again.

Exit code 100 = passed, otherwise the number of the failed check.  A plain `--aux 8M`
RamWorks must fail check 1 (it does not answer the chip detect).

## Build and run

    make
    make test          # a2run, IIe and enhanced IIe: test-ramworks (--aux 64K..8M)
                       # and test-ram2e (--aux RAM2E, three power-ons, plain 8M)
