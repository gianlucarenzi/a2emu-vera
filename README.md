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
  **`PR#3` (80 columns) needs `A2_AUX=64`**: the IIe defaults to no aux memory, and at reset the IIe
  firmware enables its own 80-column code at $C3xx only if it finds aux memory; otherwise it leaves
  the (empty) external slot 3 mapped and `PR#3` hangs, as on a real IIe without an 80-column card.
  `a2run` takes `--aux 64` for the same.
  Aux memory size: `./a2emu --aux SIZE disk.po` (overrides `A2_AUX`; `a2run --aux SIZE`), SIZE =
  `0`, `64K` (extended 80-column card), `512K`, `1M`, `4M` or `8M`.  512K-8M fit a RamWorks
  III-compatible card (8M = RAM2E II): a write to `$C073` selects the 64 KB aux bank, reset selects
  bank 0, the 80-column screen always shows bank 0.  The RAM2E's own configuration commands are not
  emulated.  E.g. `A2_MODEL=iiee scripts/a2emu-vera --aux 8M disk.po`.
- VERA slot: `A2_VERA_SLOT=<1-7>` (default 4; `a2run --vera-slot N`), e.g. `A2_VERA_SLOT=5 ./a2emu disk.po`
  puts the card at $C500-$C51F.  Slot 6 always holds the Disk II and slot 7 the hard disk when one is
  inserted: their firmware wins the reads there, so the emulator warns that the VERA is unreachable.
  Programs find the slot at run time with `examples/vera/vera.c` (`VERA_DETECTION_TEST_REPORT.md`).
- Windows: `scripts/a2emu-vera [args]` starts this tree's a2emu and places its windows with the
  window manager (`scripts/a2emu-vera-windows`, wmctrl): Apple II top-left, VERA bottom-right of the
  primary display, computed from xrandr and the panels, for any resolution or number of displays.
- `make install` copies binaries, `assets/` and `rom/` to `~/.local/share/a2emu-vera` and puts `a2emu`/`a2run`/`a2fs` in `~/bin` (override with `DATADIR=`/`BINDIR=`); `make uninstall` removes them but keeps the installed `rom/`.
- Keys: F9 warp, F10 pause, F11 reset, F12 help.
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
- `examples/hello` (`make run`) and `examples/fstest` (`make test`: reads two files, writes a
  third and reads it back, then checks it from the host) use the SDK from this tree.
- `a2run --help` lists the headless options (`--hd`, `--frames`, `--wav`, `--ymlog`, `--ym on`, ...).
- The VERA's PSG (and PCM) is always there; the A2VERA's YM2151 at VERA base + $20 is optional and absent by default: `A2_YM=1` (or `a2run --ym on`) fits it.  YM2151 core: ymfm (BSD-3, `extern/ymfm`).

A2VERA code: @fletto666 
YMFM integration: @sandlbn
