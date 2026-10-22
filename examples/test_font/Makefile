# test_font.dsk: every glyph of the VERA text font (port of VERA_C64 test_font).
#   make            -> test_font.dsk (VT_MODE=8: 80x30; 4: 40x30; 6: 80x60)
#   make test       -> run headless in a2run, VERA screenshot in test_font.png
#   make run        -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65    ?= cl65
SDK     ?= ../../sdk
VERA    ?= ../vera
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
VT_MODE ?= 8
A2RUN   ?= a2run

all: test_font.dsk

test_font.bin: test_font.c $(VERA)/vera.c $(VERA)/vera.h $(VERA)/vera_io.s $(VERA)/vtext.c $(VERA)/vtext.h $(VERA)/vtfont.s $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -I $(VERA) -DVT_MODE=$(VT_MODE) \
	  --asm-define VT_MODE=$(VT_MODE) --bin-include-dir $(VERA) \
	  -o $@ test_font.c $(VERA)/vera.c $(VERA)/vera_io.s $(VERA)/vtext.c $(VERA)/vtfont.s

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

test_font.dsk: test_font.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ test_font.bin

test: test_font.dsk
	$(A2RUN) --model $(MODEL) --disk1 test_font.dsk --no-writeback --frames 600 --text --quiet \
	  --shot-exit test_font.png --shot-full

run: test_font.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu test_font.dsk

clean:
	rm -f *.o $(VERA)/*.o *.bin test_font.dsk test_font.png

.PHONY: all test run clean
