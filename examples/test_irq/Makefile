# test_irq.dsk: VERA IRQ hook test (port of VERA_C64 test_irq).
#   make            -> test_irq.dsk
#   make test       -> run headless in a2run, VERA screenshot in test_irq.png
#   make run        -> boot it in a2emu
# Outside this tree, with the emulator installed: SDK := $(shell a2fs sdk), A2FS = a2fs
CL65    ?= cl65
SDK     ?= ../../sdk
VERA    ?= ../vera
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
A2RUN   ?= a2run

all: test_irq.dsk

test_irq.bin: test_irq.c $(VERA)/vera.c $(VERA)/vera.h $(VERA)/vera_io.s $(VERA)/vera_irq.s $(VERA)/vera_irq.h $(VERA)/delay.s $(SDK)/a2emu-vera.cfg
	$(CL65) -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -I $(VERA) \
	  -o $@ test_irq.c $(VERA)/vera.c $(VERA)/vera_io.s $(VERA)/vera_irq.s $(VERA)/delay.s

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

test_irq.dsk: test_irq.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ test_irq.bin

test: test_irq.dsk
	$(A2RUN) --model $(MODEL) --disk1 test_irq.dsk --no-writeback --frames 900 --text --quiet \
	  --shot-exit test_irq.png --shot-full

run: test_irq.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu test_irq.dsk

clean:
	rm -f *.o $(VERA)/*.o *.bin test_irq.dsk test_irq.png

.PHONY: all test run clean
