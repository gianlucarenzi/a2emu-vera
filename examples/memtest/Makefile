# memtest.dsk: VERA memory test, from a self-booting floppy.
#   make        -> memtest.dsk
#   make test   -> run headless in a2run
#   make run    -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65  ?= cl65
SDK   ?= ../../sdk
A2FS  ?= python3 ../../tools/a2fs.py
MODEL ?= iiee

all: memtest.dsk

memtest.bin: memtest.c $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -o $@ memtest.c

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

memtest.dsk: memtest.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ memtest.bin

test: memtest.dsk
	a2run --model $(MODEL) --disk1 memtest.dsk --no-writeback --frames 1500 --text --quiet

run: memtest.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu memtest.dsk

clean:
	rm -f *.o *.bin memtest.dsk

.PHONY: all test run clean
