# rbla2/ -- VERA Module RBL-A2 card files

Copies from the card firmware (repository Vera-Module-RBL-A2, `firmware/MCU/include/`),
so that the emulator runs the same register model and ROM as the card's ESP32:

- `a2_slinky.h` -- Apple II Memory Expansion Card ("Slinky") registers, `$C0nF` VERA map
- `prodos_format.h` -- the empty ProDOS volume `/SLINKY` written at start
- `slinky_rom.h` -- the `$Cn00-$CnFF` slot ROM, generated from `firmware/MCU/6502/src/slinky_rom.s`

Copy them again after a change there (`make -C firmware/MCU/6502` regenerates `slinky_rom.h`).
