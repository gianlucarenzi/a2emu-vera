# ramworks -- IIe aux memory banking (RamWorks III / RAM2E II)

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

## Build and run

    make
    make test          # a2run: IIe and enhanced IIe, --aux 64K 512K 1M 4M 8M
