# memtest.dsk: IIe aux RAM + VERA registers + VRAM test, border = every byte tested.
#   make            -> memtest.dsk
#   make test       -> run headless in a2run, VERA screenshot in memtest.png
#   make run        -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65    ?= cl65
SDK     ?= ../../sdk
VERA    ?= ../vera
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
A2RUN   ?= a2run

all: memtest.dsk

memtest.bin: memtest.c $(VERA)/vera.c $(VERA)/vera.h $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -I $(VERA) \
	  -o $@ memtest.c $(VERA)/vera.c

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

memtest.dsk: memtest.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ memtest.bin

test: memtest.dsk
	$(A2RUN) --model $(MODEL) --aux 64 --disk1 memtest.dsk --no-writeback --frames 2400 --text --quiet \
	  --shot-exit memtest.png --shot-full

run: memtest.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu memtest.dsk

clean:
	rm -f *.o $(VERA)/*.o *.bin memtest.dsk memtest.png

.PHONY: all test run clean
