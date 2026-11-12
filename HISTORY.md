# History

Development log of this fork: what was done, why, how it was checked and what was
left out.  Newest first.  The README's "Changes from upstream" table is the short
version; this file keeps the reasoning, the sources and the test evidence.

## 2026-11-12 -- VERA Module RBL-A2: VERA + Slinky RAM disk in one slot

What: `A2_VERA_CARD=rbl` (`a2run --vera-card rbl`) models the VERA Module RBL-A2
(repository Vera-Module-RBL-A2, `A2-SLINKY.md`): in the VERA's slot n, a 256 KB
Apple II Memory Expansion Card ("Slinky") and a ProDOS slot ROM besides the VERA.

Why: the card's expanded memory must work with ProDOS out of the box, which
needs a slot ROM with the block-device signature at `$Cn01/03/05/07`, where the
A2VERA's registers are.  The card shows the ROM after a reset (ProDOS scans the
slots at boot) and the VERA once `$C0nF` bit 0 = 1; the ProDOS driver sits at
`$Cn33`, above the VERA window, so it keeps working with the VERA in.

How (`a2core.c`, shared by a2emu and a2run):

- Registers `$C0n0-$C0nF` as the Apple card (checked against MAME's
  `a2memexp.cpp`): 24-bit address with auto-increment on `$C0n3`, `$C0n2`
  reads OR `$F0`, `$C0n4-$C0nF` plain registers; reset clears them.
- `$Cn00-$CnFF`: the ROM (writes ignored), except `$Cn00-$Cn1F` with the VERA
  in; the fetch mirror holds the ROM.  Slots 6 and 7-with-hard-disk keep their
  own cards, as for the A2VERA (the existing warning).
- The register model, the ProDOS formatter and the ROM image are copies of the
  card firmware's `a2_slinky.h`, `prodos_format.h`, `slinky_rom.h` (`rbla2/`),
  so the emulator runs the same code as the ESP32.
- `examples/vera/vera.c`: a slot with the ProDOS signature is still skipped,
  unless `"VERAS"` reads at `$CnF0`: then `$C0nF = 1` and the usual probe.
  VERA_COMMON's `plat_apple2.s` start-up does the same for libvera programs.
- Not in snapshots (neither are the VERA and `--xmem`).

Checked (a2run, ProDOS 2.4.3 from the AppleWin repository, NMOS dummy reads on):

- `examples/test_font`, `--vera-card rbl`: found in slots 1-5 and 7 on the ][+,
  IIe and enhanced IIe; slot 6 and slot 7 with `--hd`: not found, with the
  warning.  The A2VERA table (`VERA_DETECTION_TEST_REPORT.md`) is unchanged.
- Bitsy Bye lists `S4,D1:/SLINKY`; BASIC.SYSTEM `BSAVE`s a 10240-byte file
  to `/SLINKY` that reads back identical from `--rbl-dump` (][+, IIe, IIe
  enhanced); with `$C0CF = 1` the VERA answers at `$C400` (`'V'`, 48) and
  ProDOS keeps saving to `/SLINKY`; `$C0CF = 0` brings the ROM back.
- VERA_COMMON's Apple test program (`vera_require`) finds the VERA on the
  RBL-A2 (IIe, IIe enhanced); built with the previous libvera it does not
  ("VERA card not found in slot 4").

## 2026-11-09 -- No-Slot Clock with a date that never goes back

What: a Dallas DS1216E SmartWatch ("No-Slot Clock") under the IIe's CD ROM,
for libvera's `plat_rtc` (VERA_COMMON `src/apple2/rtc.s`): the dates of the
files a program writes on the VERA's microSD.

Why: the Apple had no clock, so its files all got the date the program set or
1980-01-01.  The user wanted a clock that ticks every second, is saved to a
file when the emulator closes and taken from there at the next start, so that
no date earlier than the last one is ever written.  A real card was chosen
over an emulator-only register so that the same driver works on a real IIe
with the clock.

How (`a2core.c`, shared by a2emu and a2run):

- Seen through the internal ROM at `$C100-$CFFF` (data reads go through
  `readMemSlow`): 64 reads with A2 = 0 write the recognition pattern
  (C5 3A A3 5C C5 3A A3 5C, LSB first) on A0, a read with A2 = 1 starts it
  again; then 64 reads with A2 = 1 give the 8 BCD registers on D0 (or 64
  with A2 = 0 set them).
- The time: the saved date (`A2_NSC_FILE`, default
  `~/.config/a2emu-vera/nsc-clock.txt`, `YYYY-MM-DD HH:MM:SS`; 1980-01-01
  00:00:00 without one) plus the emulated CPU cycles / 1,020,484, kept up in
  `cpu_vera_step` (the 32-bit `ticks` wraps after ~70 min); saved with
  `atexit`.  A program setting it moves it forward only.  IIe only,
  `A2_NSC=0` removes it.
- Not modelled: the EF ROM socket, the oscillator / reset bits, 12-hour mode.

Checked with a2run and VERA_COMMON's `tests/test_sdwrite.c` (the file dates
read on the PC with mtools): no file -> 1980-01-01 00:00:03 at the first read,
00:00:15 saved after 900 frames; the next run went on from 00:00:15 (saved
00:00:30); a file with 2026-11-09 12:30:00 -> the files dated 12:30, 12:30:15
saved; `A2_NSC=0` -> no clock, the date set by the program, the file not
touched.  Not checked: a program setting the clock (forward only), the GUI
(same core).

## 2026-10-22 -- IIe aux memory: RamWorks, RAM2E II, double-res

Branch `iie-video-rom` on github.com/gianlucarenzi/a2emu-vera (remote `gianluca`),
fast-forwarded into `main` there.  `origin` (sandlbn/a2emu-vera) is upstream and
never pushed to.

### Commits

| Commit | Repo | What |
|---|---|---|
| ed1ff1e | a2emu-vera | RamWorks III aux banking up to 8 MB; `--aux` takes sizes |
| 94c3f19 | a2emu-vera | Window titles: IIe model in the main one, `VERA [A2]`; one placement script for all emulators |
| b9ffdf4d54 | VICE (`vera-io1`) | VERA window title `VERA [C64]` |
| 845bff9f | atari800 (`master`) | VERA window title `VERA [A8]` |
| 7d7d653 | a2emu-vera | RAM2E II configuration commands (`--aux RAM2E`) |
| f3a87aa | a2emu-vera | README: changelog from upstream, keys, examples, a2run options |
| 4c14713 | a2emu-vera | Double hi-res and double lo-res, only with an aux card |
| 01bd1a8 | a2emu-vera | README: changelog hash for double-res |

Also: `examples/vera_demo/vera_demo_debug.c` and `vera_demo_readback.c` (untracked,
temporary L0_CONFIG/VERA-state debugging, not in the Makefile) were deleted; local
`main` now tracks `gianluca/main`; `make install` was run.

### 1. RamWorks III banking (ed1ff1e)

Question that started it: is the emulator compatible with the RAM2E II (GW4203C)?
It was not: only one 64 KB aux bank, no `$C073` handling, no double-res.

- `g_iie_aux` became the number of 64 KB aux banks: 0 none, 1 extended 80-column
  card, 2-128 RamWorks.  Bank 0 is static, the others are allocated at the first
  switch (`struct auxbank`: 48 KB RAM + 12 KB LC + 4 KB LC bank 2).
- A write to the bank register picks the bank (bits 0-6, modulo the banks fitted).
  ALTZP works by swapping ZP/stack/LC with aux, so a bank switch undoes the swap,
  remaps, and redoes it.  The 80-column video always shows bank 0, as on the RamWorks.
  Reset selects bank 0.
- `--aux` / `A2_AUX` / `a2emu --aux SIZE`: `0`, `64K`, `512K`, `1M`, `4M`, `8M`
  (K/KB/M/MB, any case; a bare number is KB so the old `64` still works); anything
  else is an error.
- Test: `examples/ramworks/ramworks.s` writes each bank's number through RAMWRT, ALTZP
  and the aux LC, reads back, exits with the bank count: 1/8/16/64/128 for
  64K/512K/1M/4M/8M on the IIe and the enhanced IIe; main RAM untouched.  Without aux
  (and on the ][+) it exits 200, as expected: ALTZP then maps nothing, the writes
  land in main.

### 2. Window titles and one placement script (94c3f19 + VICE + atari800)

- The disk-insert title was always `Reinette ][+`; `a2_model_name()` now gives
  `Reinette ][+` / `Reinette //e` / `Reinette //e (enhanced)`.
- VERA windows: `VERA [A2]` (a2emu), `VERA [C64]` (VICE, `uivera.c`), `VERA [A8]`
  (atari800, `vera_video.c`, was `VeraX16 Video`).  VICE and atari800 were rebuilt
  and installed in `~/bin`.
- `scripts/a2emu-vera-windows` finds any `VERA [` window and takes the other window
  of the same process as the emulator, so it places all three with no options
  (checked: a2emu, `x64sc -vera`, `atari800 -verax16`).

### 3. RAM2E II configuration commands (7d7d653)

Not from memory: modelled on Garrett's Workshop's own sources.

- HDL: <https://git.applefritter.com/Apple-2-HW/RAM2E> (`CPLD/RAM2E.v`, `CPLD/UFM-MAX.v`,
  release C.1.1).
- Utility: <https://github.com/garrettsworkshop/GWRAM.SYSTEM> (`ram2e_hal.c`,
  `ram2e_hal_max.c`, `ram2e.c`).

What the HDL does, and what is emulated:

- Bank register: any write to `$C07x` with A0=1, A3=0 (`$C071/73/75/77`).  Bank =
  byte AND capacity mask.  RWBank bit 7 is a column bit the 8 MB SDRAM ignores, so
  banks 128-255 mirror 0-127.
- Command sequence on that register: `FF 00 55 AA C1 AD`, then the command (CS 6),
  then its argument (CS 7).  A wrong byte resets it; so do 8 or more CPU cycles with
  no register write (`CmdTout`).  Emulated with the CPU tick counter: reset when two
  writes are 9 or more ticks apart.
- Commands: `FF` MAX chip detect, `F0` LED present, `F1` rev. C, `E3` LED get (if on):
  the next register write latches bank `$FF` instead of the data (how software tells
  a RAM2E from a plain RamWorks).  `E0` capacity mask = arg, `E2` LED = arg bit 0.
  `EA` UFM bitbang (bit 7 clocks in bit 6, 16-bit register, MSB first), `EE` erase,
  `EF` program -- once per power-on, as in the HDL.
- Settings word: mask (8 bits, `$80` means `$FF`), LED bit (LED = bit XOR mask bit 7),
  seven 1s; `$FFFF` = erased (defaults: mask `$7F`, LED off).  Stored as 2 bytes in
  `ram2e.nvm` next to `rom/` (`A2_RAM2E_NVM`; `a2run --ram2e-nvm`, default not saved).
- No reset line on the card: bank, mask and LED survive a reset.
- Emulated chip: Altera MAX ("typeA"), the first one GWRAM.SYSTEM probes.  The MachXO2
  ("typeC", `EC`/`ED` EFB access) and SPI flash variants are not.

Tests:

- `examples/ramworks/ram2e.s`, three power-ons (`make test-ram2e`): detection,
  LED, masks `$07/$00/$FF` = 8/1/128 banks, a command with 7 idle cycles works and
  with 9 times out, settings saved (`$0FFF` = mask `$0F` + LED on), reloaded at the
  next power-on, erased, defaults back.  A plain `--aux 8M` fails check 1 (no answer
  to the chip detect).  18/18 with `make test`.
- Mutation check: timeout threshold too long or too short, mask ignored -- each
  caught by the intended check (12, 11, 8).
- The real GWRAM.SYSTEM (`bin/GWRAM.po` from its repo, run only inside the
  emulator, headless via `a2run --input`): "Current RAM2E capacity: 8192 kB",
  `V` x5 shows `typeA revC`, `L` + Open-Apple+2 saves 512 KB + LED on, next power-on
  shows 512 kB / LED enabled, `R` x25 erases, next power-on 8192 kB / LED disabled.

### 4. Changelog and main (f3a87aa)

- README: "Changes from upstream" table (all 35 commits since upstream 9cddaec,
  checked by script), F1-F7 keys, `examples/ramworks`, the header of `a2run.c`
  lists the options (`a2run --help` does not exist).
- There is no `master` branch: `main` was fast-forwarded to `iie-video-rom` and
  pushed to `gianluca/main`; local `main` tracks `gianluca/main`.

### 5. Double hi-res / double lo-res (4c14713)

Only when an aux card is fitted.  Source for the colours and the mode condition:
MAME `src/mame/apple/apple2video.cpp`
(<https://raw.githubusercontent.com/mamedev/mame/master/src/mame/apple/apple2video.cpp>).

- `DHIRES`: set by any access to `$C05E` (AN3 off), cleared by `$C05F` and reset;
  IIe only.
- Active when graphics + 80COL + DHIRES and aux memory is fitted (as MAME:
  `m_dhires && m_80col && m_aux_ptr`); otherwise the normal modes are drawn.
- DHGR: 560 dots, bytes alternating aux/main (7 dots each); every aligned group of
  4 dots (first dot = bit 0) is a lo-res colour rotated left by one bit
  (`rotl4(n, 1)`).  DLGR: 80 x 48, aux block (rotated left by one) then main block,
  7 dots each.  RGB-perfect colours, no NTSC artifacts.
- Every graphics cache is cleared when the graphics mode changes (before, the old
  mode could stay on screen for up to half a second).
- Not done: `$C07F` RDDHIRES read, IOUDIS.

Tests (GUI, windows placed, screenshots checked by script against the palette):

- `examples/dblres`: 16 bars in DHGR, DLGR, then plain HGR with DHIRES off.
- IIe enhanced + 64K, IIe NMOS + 64K, IIe enhanced + RAM2E: every bar centre has
  exactly its palette colour, in both double modes.
- No aux (`--aux 0`) and ][+: 0 pixels differ from the plain HGR screen.
- Regressions: `examples/ramworks` 18/18, `examples/memtest` OK.

### Lessons (how to work on this tree)

- Place the emulator windows in the same command that launches it, by PID:
  `a2emu ... & P=$!; scripts/a2emu-vera-windows --pid $P`.  Background only the
  emulator: `make x && a2emu ... &` puts the whole chain in a subshell, `$!` is the
  wrong PID and the windows stay centered.
- Never stop it with `pkill -f <text of the command line>`: it also kills the shell
  that runs the command.  Use the PID or `pkill -x a2emu`.
- Test aux changes on the ][+, the IIe (NMOS 6502) and the enhanced IIe.
- Check unfamiliar hardware against its HDL or a reference emulator, not memory.
