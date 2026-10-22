# fstest.dsk: A2FS read/write test on a self-booting Disk II image, no DOS needed.
#   make        -> fstest.dsk (with DATA4K and DATA8K)
#   make test   -> run it headless in a2run (DISKTIME=1: timed disk), then check OUT3K
#                  from the host with fs.py
#   make run    -> boot it in a2emu (enhanced IIe; MODEL=plus for the ][+)
CL65  ?= cl65
LOAD   = 0x0900
HIMEM  = 0x9400
MODEL ?= iiee
# a2run disk model: 1 = rotational timing (like a real drive), 0 = simple
DISKTIME ?= 1
SRC    = fstest.c a2fs.c rwts.s
# -Cl: static locals, the fastest here
COPT  ?= -Osir -Cl

all: fstest.dsk

fstest.bin: $(SRC) a2fs.h apple2-fstest.cfg
	$(CL65) -t apple2 -C apple2-fstest.cfg $(COPT) -m fstest.map -o $@ $(SRC)

boot.bin: ../hello/boot.s fstest.bin
	$(CL65) -t none --start-addr 0x800 -o $@ \
	  --asm-define NSECT=$$(( ($$(stat -c %s fstest.bin) + 255) / 256 )) \
	  --asm-define LOAD=$(LOAD) --asm-define HIMEM=$(HIMEM) ../hello/boot.s

data4k.bin data8k.bin:
	python3 -c "import random; random.seed('$@'); \
	  open('$@', 'wb').write(bytes(random.randrange(256) for _ in range($(if $(findstring 4k,$@),4096,8192))))"

fstest.dsk: boot.bin fstest.bin data4k.bin data8k.bin fs.py
	python3 fs.py mkimage $@ boot.bin fstest.bin DATA4K=data4k.bin DATA8K=data8k.bin

# a2run writes the floppy back at exit: work on a copy, then read OUT3K on the host
test: fstest.dsk
	cp fstest.dsk test.dsk
	A2_DISKTIME=$(DISKTIME) a2run --model $(MODEL) --disk1 test.dsk --frames 3000 --text --quiet | grep -v '^|  *|$$'
	python3 fs.py ls test.dsk
	python3 fs.py get test.dsk OUT3K out3k.bin

run: fstest.dsk
	A2_MODEL=$(if $(filter plus,$(MODEL)),,$(MODEL)) a2emu fstest.dsk

clean:
	rm -f *.o *.bin *.map fstest.dsk test.dsk

.PHONY: all test run clean
