# test_scrolling Makefile for Apple II
SDK = ../../sdk
TOOLS = ../../tools

all: test_scrolling.dsk

test_scrolling.bin: test_scrolling.c
	cl65 -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -o test_scrolling.bin test_scrolling.c

test_scrolling.dsk: test_scrolling.bin
	python3 $(TOOLS)/a2fs.py mkimage test_scrolling.dsk test_scrolling.bin

clean:
	rm -f test_scrolling.o test_scrolling.bin test_scrolling.dsk

.PHONY: all clean
