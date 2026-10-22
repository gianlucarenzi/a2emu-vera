# ramworks.bin: IIe aux banking test (RamWorks III / RAM2E II, a write to $C073).
# ram2e.bin: the RAM2E II's configuration commands (detect, LED, capacity mask,
# command timeout, settings saved over a power cycle).
#   make            -> ramworks.bin (raw, runs at $0400), ram2e.bin (raw, $0800)
#   make test       -> both, headless in a2run, on the IIe and the enhanced IIe
CL65    ?= cl65
A2RUN   ?= ../../a2run
MODELS  ?= iie iiee
# --aux size : banks expected
SIZES   ?= 64K:1 512K:8 1M:16 4M:64 8M:128

all: ramworks.bin ram2e.bin

%.bin: %.s
	$(CL65) -t none -o $@ $<

phase%.bin:
	printf "\\$$(printf %o $*)" > $@

RUN2E = $(A2RUN) --vera-slot 0 --load ram2e.bin@800 --pc 800 --cycles 20000000 --quiet

test: test-ramworks test-ram2e

# phase 0 on a fresh card, 1 and 2 on the next power-ons with the settings it saved;
# a plain 8 MB RamWorks must fail check 1 (no answer to the MAX chip detect)
test-ram2e: ram2e.bin phase0.bin phase1.bin phase2.bin
	@fail=0; for m in $(MODELS); do rm -f ram2e-test.nvm; for p in 0 1 2; do \
	  $(RUN2E) --model $$m --aux RAM2E --ram2e-nvm ram2e-test.nvm --load phase$$p.bin@300; got=$$?; \
	  if [ $$got = 100 ]; then echo "ok   $$m RAM2E phase $$p"; \
	  else echo "FAIL $$m RAM2E phase $$p: check $$got"; fail=1; fi; \
	done; \
	$(RUN2E) --model $$m --aux 8M --load phase0.bin@300; got=$$?; \
	if [ $$got = 1 ]; then echo "ok   $$m 8M RamWorks is not a RAM2E"; \
	else echo "FAIL $$m 8M RamWorks: exit $$got, want 1"; fail=1; fi; \
	done; rm -f ram2e-test.nvm; exit $$fail

test-ramworks: ramworks.bin
	@fail=0; for m in $(MODELS); do for s in $(SIZES); do \
	  size=$${s%%:*}; want=$${s##*:}; \
	  $(A2RUN) --model $$m --aux $$size --vera-slot 0 --load ramworks.bin@400 --pc 400 \
	    --cycles 20000000 --quiet; got=$$?; \
	  if [ $$got = $$want ]; then echo "ok   $$m --aux $$size: $$got banks"; \
	  else echo "FAIL $$m --aux $$size: exit $$got, want $$want"; fail=1; fi; \
	done; done; exit $$fail

clean:
	rm -f *.o *.bin ram2e-test.nvm

.PHONY: all test test-ramworks test-ram2e clean
