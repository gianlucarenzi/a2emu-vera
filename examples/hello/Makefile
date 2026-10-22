# hello.dsk: a cc65 C program on a self-booting Disk II image, no DOS needed.
#   make        -> hello.dsk
#   make run    -> boot it in a2emu (enhanced IIe; MODEL=plus for the ][+)
CL65  ?= cl65
LOAD   = 0x0900
MODEL ?= iiee

all: hello.dsk

hello.bin: hello.c apple2-boot.cfg
	$(CL65) -t apple2 -C apple2-boot.cfg -O -o $@ hello.c

# the boot sector needs the payload size in sectors
boot.bin: boot.s hello.bin
	$(CL65) -t none --start-addr 0x800 -o $@ \
	  --asm-define NSECT=$$(( ($$(stat -c %s hello.bin) + 255) / 256 )) \
	  --asm-define LOAD=$(LOAD) boot.s

hello.dsk: boot.bin hello.bin mkdsk.py
	python3 mkdsk.py boot.bin hello.bin $@

run: hello.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu hello.dsk

clean:
	rm -f *.o *.bin hello.dsk

.PHONY: all run clean
