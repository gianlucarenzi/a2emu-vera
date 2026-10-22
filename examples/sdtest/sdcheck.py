#!/usr/bin/env python3
"""sdcheck.py make|check IMAGE -- the SD card image for sdtest.c.

make: 1 MB image whose block n holds sdtest.c's fill(n) pattern.
check: blocks 1, 4, 5 hold what sdtest wrote (fill(1001), fill(1004), fill(1005)),
       the others are untouched."""
import sys

BLOCKS = 2048


def fill(lba):
    return bytes(((i * 13 + lba * 7) & 0xFF) ^ 0x5A for i in range(512))


def main(cmd, path):
    if cmd == 'make':
        open(path, 'wb').write(b''.join(fill(n) for n in range(BLOCKS)))
        return
    img = open(path, 'rb').read()
    want = {1: 1001, 4: 1004, 5: 1005}
    bad = [n for n in range(BLOCKS) if img[n * 512:(n + 1) * 512] != fill(want.get(n, n))]
    if bad:
        sys.exit(f'sdcheck: blocks differ: {bad[:10]}')
    print(f'sdcheck: {path}: blocks 1, 4, 5 written, the other {BLOCKS - 3} untouched: ok')


if __name__ == '__main__':
    if len(sys.argv) != 3 or sys.argv[1] not in ('make', 'check'):
        sys.exit(__doc__)
    main(*sys.argv[1:])
