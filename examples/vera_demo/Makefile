# vera_demo.dsk: a 320x240 8bpp bitmap on the VERA layer 0, border cycling every frame.
#   make            -> vera_demo.dsk
#   make test       -> run headless in a2run, VERA screenshot in vera_demo.png
#   make run        -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65    ?= cl65
SDK     ?= ../../sdk
VERA    ?= ../vera
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
A2RUN   ?= a2run

all: vera_demo.dsk

vera_demo.bin: vera_demo.c $(VERA)/vera.c $(VERA)/vera.h $(VERA)/vera_io.s $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -I $(VERA) \
	  -o $@ vera_demo.c $(VERA)/vera.c $(VERA)/vera_io.s

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

vera_demo.dsk: vera_demo.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ vera_demo.bin

test: vera_demo.dsk
	$(A2RUN) --model $(MODEL) --disk1 vera_demo.dsk --no-writeback --frames 600 --text --quiet \
	  --shot-exit vera_demo.png --shot-full

run: vera_demo.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu vera_demo.dsk

clean:
	rm -f *.o $(VERA)/*.o *.bin vera_demo.dsk vera_demo.png

.PHONY: all test run clean
