# test_scrolling.dsk: VERA gradient + VSYNC-locked scroll (port of VERA_C64 test_gradient_scroll).
#   make            -> test_scrolling.dsk (VT_MODE=8: 80x30; 4: 40x30; 6: 80x60)
#   make test       -> run headless in a2run, VERA screenshot in test_scrolling.png
#   make run        -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65    ?= cl65
SDK     ?= ../../sdk
VERA    ?= ../vera
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
VT_MODE ?= 8
A2RUN   ?= a2run

all: test_scrolling.dsk

test_scrolling.bin: test_scrolling.c $(VERA)/vera.c $(VERA)/vera.h $(VERA)/vtext.c $(VERA)/vtext.h $(VERA)/vtfont.s $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -I $(VERA) -DVT_MODE=$(VT_MODE) \
	  --asm-define VT_MODE=$(VT_MODE) --bin-include-dir $(VERA) \
	  -o $@ test_scrolling.c $(VERA)/vera.c $(VERA)/vtext.c $(VERA)/vtfont.s

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

test_scrolling.dsk: test_scrolling.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ test_scrolling.bin

test: test_scrolling.dsk
	$(A2RUN) --model $(MODEL) --disk1 test_scrolling.dsk --no-writeback --frames 600 --text --quiet \
	  --shot-exit test_scrolling.png --shot-full

run: test_scrolling.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu test_scrolling.dsk

clean:
	rm -f *.o $(VERA)/*.o *.bin test_scrolling.dsk test_scrolling.png

.PHONY: all test run clean
