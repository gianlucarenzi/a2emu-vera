# sdtest.dsk: the SD card on the VERA's SPI port, from a self-booting floppy.
#   make        -> sdtest.dsk
#   make test   -> fresh sd.img, run headless in a2run, then check the image from the host
#   make run    -> boot it in a2emu with sd.img as the SD card
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65  ?= cl65
SDK   ?= ../../sdk
A2FS  ?= python3 ../../tools/a2fs.py
MODEL ?= iiee

all: sdtest.dsk

sdtest.bin: sdtest.c $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -o $@ sdtest.c

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

sdtest.dsk: sdtest.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ sdtest.bin

test: sdtest.dsk
	python3 sdcheck.py make sd.img
	a2run --model $(MODEL) --disk1 sdtest.dsk --no-writeback --sdcard sd.img --frames 1500 --text --quiet | grep -v '^|  *|$$'
	python3 sdcheck.py check sd.img

run: sdtest.dsk
	[ -f sd.img ] || python3 sdcheck.py make sd.img
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) A2_SDCARD=sd.img a2emu sdtest.dsk

clean:
	rm -f *.o *.bin sdtest.dsk sd.img

.PHONY: all test run clean
