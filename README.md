# a2emu — Apple II + A2VERA emulator

Reinette II plus (Apple ][+ / IIe, 65C02) with an A2VERA card: VERA video, PSG and the YM2151 at VERA base + $20.

- `make` builds `a2emu` (SDL2 window + audio) and `a2run` (headless test runner); `make windows` builds `a2emu.exe`.
- Put the Apple ROMs in `rom/` (`appleII+.rom`, `diskII.rom`, IIe 342-0135/0134 or 342-0303/0304, IIe video ROM 342-0133 or 342-0265 for lowercase/80 columns); they are not included.
- ROM source: the images used during development came from the Asimov archive mirror,
  <https://mirrors.apple2.org.za/ftp.apple.asimov.net/emulators/rom_images/>
  (`apple_ii+_rom.zip`, `Apple Disk II 16 Sector Interface Card ROM P5 - 341-0027.bin`, and the
  IIe CPU and video ROMs from `APPLE Computer and Peripheral Card Roms Collection.zip`).
  **The Apple ROMs are copyrighted by Apple: downloading them is not legal unless you actually own
  the original hardware.** If you do, dumping the chips of your own machine is the clean way to get them.
- Run: `A2_MODEL=iie ./a2emu disk.po` (from this directory); `iiee` = enhanced IIe, `A2_AUX=64` adds the extended 80-column card.
  **`PR#3` (80 columns) needs `A2_AUX=64`**: the IIe defaults to no aux memory, and at reset the IIe
  firmware enables its own 80-column code at $C3xx only if it finds aux memory; otherwise it leaves
  the (empty) external slot 3 mapped and `PR#3` hangs, as on a real IIe without an 80-column card.
  `a2run` takes `--aux 64` for the same.
- `make install` copies binaries, `assets/` and `rom/` to `~/.local/share/a2emu-vera` and puts `a2emu`/`a2run` launchers in `~/bin` (override with `DATADIR=`/`BINDIR=`); `make uninstall` removes them.
- Keys: F9 warp, F10 pause, F11 reset, F12 help.
- Env: `A2_WAV=out.wav` records the stereo card audio, `A2_SPEAKER=0` drops the speaker stream, `A2_FPS=1` prints the frame rate.
- `a2run --help` lists the headless options (`--hd`, `--frames`, `--wav`, `--ymlog`, `--ym off`, ...).
- YM2151 core: ymfm (BSD-3, `extern/ymfm`).

A2VERA code: @fletto666 
YMFM integration: @sandlbn
