# dblres.dsk: IIe double hi-res / double lo-res colour bars (needs aux memory).
#   make            -> dblres.dsk (self-booting, no DOS)
#   make run        -> boot it in a2emu (enhanced IIe, 64 KB aux); a key steps
#                      DHGR -> DLGR -> plain HGR (DHIRES off) -> DHGR ...
CL65    ?= cl65
SDK     ?= ../../sdk
A2FS    ?= python3 ../../tools/a2fs.py
MODEL   ?= iiee
AUX     ?= 64K

all: dblres.dsk

data.inc: gendata.py
	python3 gendata.py > $@

dblres.bin: dblres.s data.inc
	$(CL65) -t none -o $@ dblres.s

$(SDK)/boot.sector: $(SDK)/boot.s
	$(MAKE) -C ../.. sdk/boot.sector

dblres.dsk: dblres.bin $(SDK)/boot.sector
	$(A2FS) mkimage $@ dblres.bin

run: dblres.dsk
	A2_MODEL=$(MODEL) a2emu --aux $(AUX) dblres.dsk

clean:
	rm -f *.o *.bin data.inc dblres.dsk

.PHONY: all run clean
