# a2emu — Apple II + A2VERA emulator

Reinette II plus (Apple ][+ / IIe, 65C02) with an A2VERA card: VERA video, PSG and the YM2151 at VERA base + $20.

- `make` builds `a2emu` (SDL2 window + audio) and `a2run` (headless test runner); `make windows` builds `a2emu.exe`.
- Put the Apple ROMs in `rom/` (`appleII+.rom`, `diskII.rom`, IIe 342-0135/0134 or 342-0303/0304, IIe video ROM 342-0133 or 342-0265 for lowercase/80 columns); they are not included.
- Run: `A2_MODEL=iie ./a2emu disk.po` (from this directory); `iiee` = enhanced IIe, `A2_AUX=64` adds the extended 80-column card.
- `make install` copies binaries, `assets/` and `rom/` to `~/.local/share/a2emu-vera` and puts `a2emu`/`a2run` launchers in `~/bin` (override with `DATADIR=`/`BINDIR=`); `make uninstall` removes them.
- Keys: F9 warp, F10 pause, F11 reset, F12 help.
- Env: `A2_WAV=out.wav` records the stereo card audio, `A2_SPEAKER=0` drops the speaker stream, `A2_FPS=1` prints the frame rate.
- `a2run --help` lists the headless options (`--hd`, `--frames`, `--wav`, `--ymlog`, `--ym off`, ...).
- YM2151 core: ymfm (BSD-3, `extern/ymfm`).

A2VERA code: @fletto666 
YMFM integration: @sandlbn
