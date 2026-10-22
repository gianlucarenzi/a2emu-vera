# vera_demo.dsk: Simple VERA graphics demo for Apple II
#   make        -> vera_demo.dsk
#   make test   -> run headless in a2run
#   make run    -> boot in a2emu
CL65  ?= cl65
SDK   ?= ../../sdk
A2FS  ?= python3 ../../tools/a2fs.py
MODEL ?= iiee

all: vera_demo.dsk

vera_demo.bin: vera_demo.c $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -o $@ vera_demo.c

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

vera_demo.dsk: vera_demo.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ vera_demo.bin

test: vera_demo.dsk
	a2run --model $(MODEL) --disk1 vera_demo.dsk --no-writeback --frames 1500 --text --quiet

run: vera_demo.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu vera_demo.dsk

clean:
	rm -f *.o *.bin vera_demo.dsk

.PHONY: all test run clean
