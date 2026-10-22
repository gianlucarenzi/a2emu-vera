# Apple II + VERA emulator.  make            -> native (needs SDL2)
#                            make windows    -> a2emu.exe via MXE (static SDL2)
SRC    = reinette.c a2core.c puce6502.c veracore.c
CFLAGS = -O2 -Wall -std=gnu17 -I.
CXXFLAGS = -O2 -Wall -std=c++17 -I.

all: a2emu a2run
# the YM2151 (E14): ymfm's OPM core + our glue, C++
YM_OBJ = ym2151.o ymfm_opm.o
YM_HDR = ym2151.h extern/ymfm/ymfm.h extern/ymfm/ymfm_fm.h extern/ymfm/ymfm_fm.ipp extern/ymfm/ymfm_opm.h
ym2151.o: ym2151.cpp $(YM_HDR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
ymfm_opm.o: extern/ymfm/ymfm_opm.cpp $(YM_HDR)
	$(CXX) $(CXXFLAGS) -w -c -o $@ $<

a2emu: $(SRC) a2core.h ym2151.h $(YM_OBJ)
	$(CC) $(CFLAGS) -c reinette.c a2core.c puce6502.c veracore.c `pkg-config --cflags sdl2`
	$(CXX) -o $@ reinette.o a2core.o puce6502.o veracore.o $(YM_OBJ) `pkg-config --libs sdl2` -lm

# headless runner for the test gates: no SDL
CORE   = a2core.c puce6502.c veracore.c
a2run: a2run.c a2run_audio.c a2run_audio.h a2run_input.c a2run_input.h $(CORE) a2core.h ym2151.h $(YM_OBJ)
	$(CC) $(CFLAGS) -o $@ a2run.c a2run_audio.c a2run_input.c $(CORE) $(YM_OBJ) -lstdc++ -lm

MXE    = /opt/mxe/usr/bin/x86_64-w64-mingw32.static
windows: $(SRC)
	$(MXE)-g++ $(CXXFLAGS) -w -c ym2151.cpp extern/ymfm/ymfm_opm.cpp
	$(MXE)-gcc $(CFLAGS) `$(MXE)-pkg-config --cflags sdl2` -o a2emu.exe $(SRC) ym2151.o ymfm_opm.o \
	  `$(MXE)-pkg-config --libs sdl2` -lstdc++ -lm

# make install: binaries + assets/ (+ rom/ if present) in DATADIR, launchers in BINDIR
DATADIR ?= $(HOME)/.local/share/a2emu-vera
BINDIR  ?= $(HOME)/bin
install: a2emu a2run
	mkdir -p $(DATADIR)/assets $(BINDIR)
	install -m755 a2emu a2run $(DATADIR)/
	install -m644 assets/*.bmp $(DATADIR)/assets/
	if [ -d rom ] && [ rom -ef $(DATADIR)/rom ]; then :; \
	elif [ -d rom ]; then mkdir -p $(DATADIR)/rom && install -m644 rom/* $(DATADIR)/rom/; \
	else echo "note: no rom/ here, put the Apple ROMs in $(DATADIR)/rom"; fi
	for s in a2emu a2run; do \
	  sed 's|@DATADIR@|$(DATADIR)|' scripts/$$s.in > $(BINDIR)/$$s && chmod 755 $(BINDIR)/$$s; done

uninstall:
	rm -f $(BINDIR)/a2emu $(BINDIR)/a2run
	rm -rf $(DATADIR)

clean:
	rm -f a2emu a2run a2emu.exe *.o

.PHONY: all windows install uninstall clean
