#!/usr/bin/env python3
"""mkdsk.py BOOT PAYLOAD OUT.dsk -- a 140 KB DOS-order disk image with BOOT in
track 0 sector 0 and PAYLOAD from track 0 sector 1 on, in physical-sector order
(the order boot.s reads it)."""
import sys

# physical sector -> DOS logical sector, as a2core.c converts .dsk to nibbles
P2L = [0, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 15]
SIZE = 35 * 16 * 256

boot_path, payload_path, out_path = sys.argv[1:4]
boot = open(boot_path, 'rb').read()
payload = open(payload_path, 'rb').read()
if len(boot) > 256:
    sys.exit(f'mkdsk: {boot_path} is {len(boot)} bytes, the boot sector holds 256')
if 256 + len(payload) > SIZE:
    sys.exit(f'mkdsk: {payload_path} does not fit on the disk')

img = bytearray(SIZE)


def put(n, data):
    """data into the n-th physical sector, counting from track 0 sector 0"""
    track, sector = divmod(n, 16)
    off = track * 16 * 256 + P2L[sector] * 256
    img[off:off + len(data)] = data


put(0, boot)
for i in range(0, len(payload), 256):
    put(1 + i // 256, payload[i:i + 256])
open(out_path, 'wb').write(img)
