# test_fx.dsk: VERA FX coprocessor register / behaviour test + benchmarks (port of VERA_C64 test_fx).
#   make            -> test_fx.dsk
#   make test       -> run headless in a2run, VERA screenshot in test_fx.png
#   make run        -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65    ?= cl65
SDK     ?= ../../sdk
VERA    ?= ../vera
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
A2RUN   ?= a2run

all: test_fx.dsk

test_fx.bin: test_fx.c $(VERA)/vera.c $(VERA)/vera.h $(VERA)/vera_io.s $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -I $(VERA) \
	  -o $@ test_fx.c $(VERA)/vera.c $(VERA)/vera_io.s

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

test_fx.dsk: test_fx.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ test_fx.bin

test: test_fx.dsk
	$(A2RUN) --model $(MODEL) --disk1 test_fx.dsk --no-writeback --frames 1500 --text --quiet \
	  --shot-exit test_fx.png --shot-full

run: test_fx.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu test_fx.dsk

clean:
	rm -f *.o $(VERA)/*.o *.bin test_fx.dsk test_fx.png

.PHONY: all test run clean
