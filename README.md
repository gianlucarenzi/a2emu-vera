# a2emu — Apple II + A2VERA emulator

Reinette II plus (Apple ][+ / IIe, 65C02) with an A2VERA card: VERA video, PSG and the YM2151 at VERA base + $20.

- `make` builds `a2emu` (SDL2 window + audio) and `a2run` (headless test runner); `make windows` builds `a2emu.exe`.
- Put the Apple ROMs in `rom/` (`appleII+.rom`, `diskII.rom`, IIe 342-0135/0134 or 342-0303/0304, IIe video ROM 342-0133 or 342-0265 for lowercase/80 columns); they are not included.
- ROM source: the images used during development came from the Asimov archive mirror,
  <https://mirrors.apple2.org.za/ftp.apple.asimov.net/emulators/rom_images/>
  (`apple_ii+_rom.zip` -- its F800-FFFF replaced by `Apple II+ - 341-0020 - Applesoft BASIC Autostart Monitor F800 - 2716.bin` from the collection below: the image in the zip has the IRQ vector patched to $FF59 instead of $FA40, so IRQs never reach $03FE -- `Apple Disk II 16 Sector Interface Card ROM P5 - 341-0027.bin-with-D4-D7 data bits swapped.bin`
  as `diskII.rom` -- it must start with `A2 20 A0 00`: the file without that suffix is a raw chip
  dump with scrambled lines and never boots a disk -- and the IIe CPU and video ROMs from
  `APPLE Computer and Peripheral Card Roms Collection.zip`).
  **The Apple ROMs are copyrighted by Apple: downloading them is not legal unless you actually own
  the original hardware.** If you do, dumping the chips of your own machine is the clean way to get them.
- Run: `A2_MODEL=iie ./a2emu disk.po` (from this directory); `iiee` = enhanced IIe, `A2_AUX=64` adds the extended 80-column card.
  The window title shows the model (`Reinette ][+`, `Reinette //e`, `Reinette //e (enhanced)`) and the disks.
  Double-res video (IIe with aux memory only): with 80COL on and `$C05E` (AN3 off, DHIRES) the
  graphics modes become double hi-res (560 dots, 140 x 192 in 16 colours) and double lo-res (80 x 48);
  `$C05F` or a reset turns it off.  Without an aux card, and on the ][+, `$C05E` changes nothing.
  **`PR#3` (80 columns) needs `A2_AUX=64`**: the IIe defaults to no aux memory, and at reset the IIe
  firmware enables its own 80-column code at $C3xx only if it finds aux memory; otherwise it leaves
  the (empty) external slot 3 mapped and `PR#3` hangs, as on a real IIe without an 80-column card.
  `a2run` takes `--aux 64` for the same.
  Aux memory size: `./a2emu --aux SIZE disk.po` (overrides `A2_AUX`; `a2run --aux SIZE`), SIZE =
  `0`, `64K` (extended 80-column card), `512K`, `1M`, `4M`, `8M` or `RAM2E`.  512K-8M fit a RamWorks
  III-compatible card: a write to `$C071`/`$C073`/`$C075`/`$C077` selects the 64 KB aux bank, reset
  selects bank 0, the 80-column screen always shows bank 0.
  `RAM2E` is Garrett's Workshop's RAM2E II (GW4203, 8 MB, MAX CPLD version), modelled on its HDL:
  besides the bank register it takes the command sequence (`FF 00 55 AA C1 AD` command argument
  on the bank register) for chip/LED/revision detection, the capacity mask, the LED and saving the
  settings, and it has no reset line (bank and settings survive a reset).  Its utility
  `GWRAM.SYSTEM` works: capacity, LED, Apple+number to save, R x25 to erase.  The saved settings go
  to `ram2e.nvm` next to `rom/` (`A2_RAM2E_NVM=FILE`; `a2run --ram2e-nvm FILE`, default: not saved).
  E.g. `A2_MODEL=iiee scripts/a2emu-vera --aux RAM2E GWRAM.po`.
- VERA slot: `A2_VERA_SLOT=<1-7>` (default 4; `a2run --vera-slot N`), e.g. `A2_VERA_SLOT=5 ./a2emu disk.po`
  puts the card at $C500-$C51F.  Slot 6 always holds the Disk II and slot 7 the hard disk when one is
  inserted: their firmware wins the reads there, so the emulator warns that the VERA is unreachable.
  Programs find the slot at run time with `examples/vera/vera.c` (`VERA_DETECTION_TEST_REPORT.md`).
- Windows: `scripts/a2emu-vera [args]` starts this tree's a2emu and places its windows with the
  window manager (`scripts/a2emu-vera-windows`, wmctrl): Apple II top-left, VERA bottom-right of the
  primary display, computed from xrandr and the panels, for any resolution or number of displays.
  The VERA window is titled `VERA [A2]`, as `VERA [C64]` in VICE and `VERA [A8]` in atari800, so the
  same script places any of them with no options: `scripts/a2emu-vera-windows --pid $(pgrep -n x64sc)`.
- `make install` copies binaries, `assets/` and `rom/` to `~/.local/share/a2emu-vera` and puts `a2emu`/`a2run`/`a2fs` in `~/bin` (override with `DATADIR=`/`BINDIR=`); `make uninstall` removes them but keeps the installed `rom/`.
- Keys: Ctrl+F1 / Alt+F1 save the floppy in drive 1 / 2 back to its file, F2 screenshot, F3 paste the clipboard as keystrokes,
  Shift/Ctrl+F4 volume, Shift/Ctrl+F5/F6 joystick release/action speed, Shift/Ctrl+F7 zoom,
  F9 warp, F10 pause, F11 reset, F12 help.
- SD card: `A2_SDCARD=sd.img` (or `a2run --sdcard sd.img`) puts a raw image of 512-byte sectors (at least 512 KB) on the VERA's SPI port as an SDHC card in SPI mode (CMD0/1/8/9/10/12/13/16/17/18/24/25/55/58/59, ACMD13/23/41/51; written through).  The SPI follows VERA 48.0.1's `spictrl.v` clock by clock: a byte takes 16 VERA clocks, 512 with SLOW.
- VERA timing: the VERA catches up to the CPU before every register access and after every instruction (where its IRQ is sampled, as on the 6502); its audio makes a sample every 512 VERA clocks, so the PCM FIFO and AFLOW follow CPU time.  Writes to the composer/layer registers and, unless `A2_VERA_MIDLINE=0`, to VRAM/palette/sprites draw the line up to the beam first: raster effects land on the pixel (x16emu does the VRAM part only with `-midline-effects`).
- Env: `A2_WAV=out.wav` records the stereo card audio, `A2_SPEAKER=0` drops the speaker stream, `A2_FPS=1` prints the frame rate.
- `make install` also installs `a2fs` and the SDK (`a2fs sdk` prints where), for cc65 programs
  booted straight from a Disk II floppy, no DOS:
  - `a2fs mkimage prog.dsk prog.bin [NAME=FILE ...]`: bootable image (`sdk/boot.s` loads the
    program at $0900 with the Disk II PROM's sector reader) with files in A2FS, a minimal file
    system (directory on track 17, contiguous files with a CRC-16); `a2fs ls|get|put|rm` work on
    images, `a2fs --help` has the layout.
  - link with `sdk/a2emu-vera.cfg`; for files on the Apple add `sdk/a2fs.c sdk/rwts.s` and
    `#include "a2fs.h"` (`fs_start`, `fs_mount`, `fs_read`, `fs_write`, `blk_read`, `blk_write`).
    `rwts.s` reads and writes with the real drive's timing.
  - in a project: `SDK := $(shell a2fs sdk)`, then
    `cl65 -t apple2 -C $(SDK)/a2emu-vera.cfg -I $(SDK) -Osir -Cl -o prog.bin main.c $(SDK)/a2fs.c $(SDK)/rwts.s`
    and `a2fs mkimage prog.dsk prog.bin DATA=data.bin`.
- `examples/sdtest` (`make test`): SD init, CSD, single/multiple block reads and writes over SPI, checked from the host.
- `examples/vera`: cc65 helpers for the card (`vera.h` registers, `vera_detect()`, `vera_wait_vsync()`,
  `vera_w0()`/`vera_w1()`/`VERA_POKE()` for the registers with read side effects -- a store through a
  pointer is `STA (zp),Y`, which on the NMOS 6502 of the ][+ and unenhanced IIe reads the register
  first -- and the `vtext` text console) used by `examples/test_font`, `test_scrolling`, `test_maze`, `test_matrix`, `test_fx`, `test_irq` (ports of the VERA_C64 tests;
  `vera_irq.s` hooks the 6502 IRQ vector in LC RAM, `delay.s` waits in CPU cycles),
  `vera_demo` (320x240 8 bpp bitmap) and `memtest` (IIe aux RAM + VERA registers + VRAM).
- `examples/ramworks` (`make test`): IIe aux banking for every `--aux` size, and the RAM2E II's
  commands over three power-ons (detection, LED, capacity mask, command timeout, saved settings).
- `examples/dblres` (`make run`): 16 colour bars in double hi-res, then double lo-res, then plain
  hi-res with DHIRES off (a key steps through them).
- `examples/hello` (`make run`) and `examples/fstest` (`make test`: reads two files, writes a
  third and reads it back, then checks it from the host) use the SDK from this tree.
- The header of `a2run.c` lists the headless options (`--model`, `--aux`, `--hd`, `--frames`, `--keys`,
  `--input`, `--wav`, `--ymlog`, `--ym on`, ...).
- The VERA's PSG (and PCM) is always there; the A2VERA's YM2151 at VERA base + $20 is optional and absent by default: `A2_YM=1` (or `a2run --ym on`) fits it.  YM2151 core: ymfm (BSD-3, `extern/ymfm`).

## Changes from upstream

What this fork (github.com/gianlucarenzi/a2emu-vera) adds to sandlbn/a2emu-vera `main` (9cddaec):

| Area | Change | Commits |
|---|---|---|
| Build | Builds with SDL < 2.0.22; compiler warnings cleaned up | ebea2af |
| Install | `make install`/`uninstall` with `~/bin` launchers; `rom` may be a symlink to the installed ROMs; the disk tools (`a2fs`) and a cc65 SDK installed too | 5362a80, 831a817, 2fed052 |
| IIe video | TEXT drawn with the IIe video ROM (lowercase, inverse, MouseText); 80-column TEXT from aux memory, 80STORE keeps page 1 on screen | 8e869a4, 4187b00 |
| IIe aux memory | `--aux` / `A2_AUX` = `0`, `64K`, `512K`, `1M`, `4M`, `8M`: extended 80-column card or RamWorks III banking at `$C071`-`$C077` | ed1ff1e, 7d7d653 |
| RAM2E II | `--aux RAM2E`: Garrett's Workshop RAM2E II after its HDL -- command sequence, detection, capacity mask, LED, settings saved in `ram2e.nvm`; GWRAM.SYSTEM works | 7d7d653 |
| IIe double-res | Double hi-res and double lo-res when DHIRES (`$C05E`), 80COL and an aux card are on; colours as MAME; `examples/dblres` | 4c14713 |
| Disk II | Timed disk model: a write starts on the next nibble, not the one just read | 7943cab |
| VERA | Line buffers, SCANLINE, VSTART and sprite timing per the 48.0.1 HDL; audio on VERA time, IRQ per instruction, mid-line raster effects; ISR write clears bits 2:0 only; SPI per `spictrl.v` with an SD card image; 16-bit PCM fixed | 17e1791, 92f4bd4, 484a5c6, 1aa0e1e |
| VERA slot | Configurable slot (`A2_VERA_SLOT`, `--vera-slot`); runtime slot detection for programs, robust against the hard disk and the floating bus | cd7ba20, 2c17344, f96c473, 2517f88, 40c0870 |
| Windows | Close from the X of either window; cursor hidden; window titles show the model, VERA window `VERA [A2]`; `a2emu-vera-windows` places the windows of a2emu, VICE (`VERA [C64]`) and atari800 (`VERA [A8]`) | 0e12d91, cac8ca5, 494f8bd, 6a43b11, 94c3f19 |
| Examples | `hello`, `fstest` (A2FS on a Disk II, no DOS), `memtest`, `sdtest`, `vera_demo`; VERA_C64 test ports `test_font`, `test_scrolling`, `test_maze`, `test_fx`, `test_matrix`, `test_irq`; `ramworks` (aux banking + RAM2E); VERA stores without the NMOS dummy read | 52416cc, 91cbfa0, 9b9060c, 5d95fe0, eee485c, 4f65783, dc2772c, 578bd22, 1f8357b, 197860b, ed1ff1e, 7d7d653 |
| Docs | Where the ROMs came from (and that downloading them needs the hardware); `PR#3` needs aux memory; this table | a41f1aa, 4b883c2 |

A2VERA code: @fletto666 
YMFM integration: @sandlbn
