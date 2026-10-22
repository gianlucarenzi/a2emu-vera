#!/usr/bin/env python3
"""fs.py -- A2FS disk images from the host side.

A2FS is a minimal file system on a 140 KB Disk II floppy, written by rwts.s/a2fs.c:
  block n = track n // 16, physical sector n % 16 (256 bytes)
  block 0         boot sector (boot.s), blocks 1.. the program it loads
  block 272       directory (track 17, sector 0):
                    0  "A2FS"            4  next free block (u16 LE)
                    16 10 entries of 24 bytes: name[16] (NUL padded),
                       start block, length in bytes, CRC-16/CCITT-FALSE, spare (u16 LE)
  blocks 288..    file data, each file contiguous
The .dsk file is in DOS sector order, as the emulator and disk emulators expect.

  fs.py mkimage OUT.dsk BOOT PROGRAM [NAME=FILE ...]
  fs.py ls IMAGE.dsk
  fs.py get IMAGE.dsk NAME OUTFILE        (fails if the CRC does not match)
"""
import struct
import sys

P2L = [0, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 15]   # physical -> DOS logical
SIZE = 35 * 16 * 256
DIRBLK, FIRSTDATA, NENTRIES, NAMELEN = 272, 288, 10, 16


def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if crc & 0x8000 else crc << 1
            crc &= 0xFFFF
    return crc


def off(blk):
    track, sector = divmod(blk, 16)
    return track * 4096 + P2L[sector] * 256


def put(img, blk, data):
    for i in range(0, len(data), 256):
        o = off(blk + i // 256)
        img[o:o + len(data[i:i + 256])] = data[i:i + 256]


def get(img, blk, length):
    out = bytearray()
    while len(out) < length:
        o = off(blk)
        out += img[o:o + 256]
        blk += 1
    return bytes(out[:length])


def entries(img):
    d = get(img, DIRBLK, 256)
    if d[:4] != b'A2FS':
        sys.exit('fs.py: no A2FS directory')
    for i in range(NENTRIES):
        name, start, length, crc, _ = struct.unpack_from('<16sHHHH', d, 16 + 24 * i)
        if name[0]:
            yield name.rstrip(b'\0').decode(), start, length, crc


def mkimage(out, boot, prog, files):
    img = bytearray(SIZE)
    boot, prog = open(boot, 'rb').read(), open(prog, 'rb').read()
    if len(boot) > 256 or 1 + (len(prog) + 255) // 256 > DIRBLK:
        sys.exit('fs.py: boot sector or program too large')
    put(img, 0, boot)
    put(img, 1, prog)
    d = bytearray(256)
    d[:4] = b'A2FS'
    blk = FIRSTDATA
    for i, spec in enumerate(files):
        name, path = spec.split('=', 1)
        data = open(path, 'rb').read()
        if len(name) > NAMELEN or blk + (len(data) + 255) // 256 > 35 * 16:
            sys.exit(f'fs.py: {name}: name too long or disk full')
        struct.pack_into('<16sHHHH', d, 16 + 24 * i, name.encode(), blk, len(data), crc16(data), 0)
        put(img, blk, data)
        blk += (len(data) + 255) // 256
    struct.pack_into('<H', d, 4, blk)
    put(img, DIRBLK, d)
    open(out, 'wb').write(img)


def main(argv):
    if len(argv) >= 4 and argv[0] == 'mkimage':
        mkimage(argv[1], argv[2], argv[3], argv[4:])
    elif len(argv) == 2 and argv[0] == 'ls':
        for name, start, length, crc in entries(open(argv[1], 'rb').read()):
            print(f'{name:16} block {start:3}  {length:5} bytes  crc {crc:04X}')
    elif len(argv) == 4 and argv[0] == 'get':
        img = open(argv[1], 'rb').read()
        for name, start, length, crc in entries(img):
            if name == argv[2]:
                data = get(img, start, length)
                open(argv[3], 'wb').write(data)
                if crc16(data) != crc:
                    sys.exit(f'fs.py: {name}: crc {crc16(data):04X}, directory says {crc:04X}')
                print(f'{name}: {length} bytes, crc {crc:04X} ok')
                return
        sys.exit(f'fs.py: {argv[2]}: not found')
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main(sys.argv[1:])
