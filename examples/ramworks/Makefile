# ramworks.bin: IIe aux banking test (RamWorks III / RAM2E II, a write to $C073).
#   make            -> ramworks.bin (raw, runs at $0400)
#   make test       -> run it headless in a2run for every --aux size, on the IIe and the
#                      enhanced IIe: the exit code is the number of banks found
CL65    ?= cl65
A2RUN   ?= ../../a2run
MODELS  ?= iie iiee
# --aux size : banks expected
SIZES   ?= 64K:1 512K:8 1M:16 4M:64 8M:128

all: ramworks.bin

ramworks.bin: ramworks.s
	$(CL65) -t none -o $@ $<

test: ramworks.bin
	@fail=0; for m in $(MODELS); do for s in $(SIZES); do \
	  size=$${s%%:*}; want=$${s##*:}; \
	  $(A2RUN) --model $$m --aux $$size --vera-slot 0 --load ramworks.bin@400 --pc 400 \
	    --cycles 20000000 --quiet; got=$$?; \
	  if [ $$got = $$want ]; then echo "ok   $$m --aux $$size: $$got banks"; \
	  else echo "FAIL $$m --aux $$size: exit $$got, want $$want"; fail=1; fi; \
	done; done; exit $$fail

clean:
	rm -f *.o ramworks.bin

.PHONY: all test clean
