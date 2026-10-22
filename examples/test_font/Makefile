# test_font Makefile for Apple II
SDK = ../../sdk
TOOLS = ../../tools

all: test_font.dsk

test_font.bin: test_font.c
	cl65 -t apple2 -C $(SDK)/a2emu-vera.cfg -Osir -Cl -o test_font.bin test_font.c

test_font.dsk: test_font.bin
	python3 $(TOOLS)/a2fs.py mkimage test_font.dsk test_font.bin

clean:
	rm -f test_font.o test_font.bin test_font.dsk

.PHONY: all clean
