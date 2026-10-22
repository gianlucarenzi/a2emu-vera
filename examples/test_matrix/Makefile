# test_matrix.dsk: digital rain on the VERA text console (port of VERA_C64 test_matrix).
#   make            -> test_matrix.dsk (VT_MODE=8: 80x30; 4: 40x30; 6: 80x60)
#   make test       -> run headless in a2run, VERA screenshot in test_matrix.png
#   make run        -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65    ?= cl65
SDK     ?= ../../sdk
VERA    ?= ../vera
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
VT_MODE ?= 8
A2RUN   ?= a2run

all: test_matrix.dsk

test_matrix.bin: test_matrix.c $(VERA)/vera.c $(VERA)/vera.h $(VERA)/vera_io.s $(VERA)/vtext.c $(VERA)/vtext.h $(VERA)/vtfont.s $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -I $(VERA) -DVT_MODE=$(VT_MODE) \
	  --asm-define VT_MODE=$(VT_MODE) --bin-include-dir $(VERA) \
	  -o $@ test_matrix.c $(VERA)/vera.c $(VERA)/vera_io.s $(VERA)/vtext.c $(VERA)/vtfont.s

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

test_matrix.dsk: test_matrix.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ test_matrix.bin

test: test_matrix.dsk
	$(A2RUN) --model $(MODEL) --disk1 test_matrix.dsk --no-writeback --frames 600 --text --quiet \
	  --shot-exit test_matrix.png --shot-full

run: test_matrix.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu test_matrix.dsk

clean:
	rm -f *.o $(VERA)/*.o *.bin test_matrix.dsk test_matrix.png

.PHONY: all test run clean
