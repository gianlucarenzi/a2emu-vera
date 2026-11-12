/**
 * prodos_format.h  --  write an empty ProDOS volume into a RAM disk image
 *
 * Layout (ProDOS 8 Technical Reference Manual, appendix B):
 *   blocks 0-1  boot blocks: left at zero (not bootable: PR#n goes on with
 *               the next slot until a ProDOS boot block is copied there)
 *   blocks 2-5  volume directory (key block 2, linked 2-3-4-5)
 *   block  6    volume bitmap (1 bit per block, 1 = free, MSB = lowest block)
 * Dates are zero (no clock on the card).
 *
 * No ESP-IDF dependencies: also compiled by the host test (test/host).
 * Style: Allman
 */

#ifndef PRODOS_FORMAT_H
#define PRODOS_FORMAT_H

#include <stdint.h>
#include <string.h>

#define PRODOS_BLOCK        512u
#define PRODOS_VDIR_KEY     2u
#define PRODOS_VDIR_LAST    5u
#define PRODOS_BITMAP       6u

static inline void prodos_put16(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

/*
 * Format `image` (blocks * 512 bytes, blocks <= 4096 so the bitmap fits one
 * block) as an empty volume called `name` (1-15 chars, A-Z 0-9 '.', starting
 * with a letter; not checked).
 */
static inline void prodos_format(uint8_t *image, uint32_t blocks, const char *name)
{
    memset(image, 0, (size_t)blocks * PRODOS_BLOCK);

    /* Volume directory: blocks 2-5, prev/next links. */
    for (uint32_t b = PRODOS_VDIR_KEY; b <= PRODOS_VDIR_LAST; b++)
    {
        uint8_t *blk = image + b * PRODOS_BLOCK;
        prodos_put16(blk + 0, b == PRODOS_VDIR_KEY ? 0u : b - 1u);
        prodos_put16(blk + 2, b == PRODOS_VDIR_LAST ? 0u : b + 1u);
    }

    /* Volume directory header: first entry of the key block (block offset
     * $04; the offsets below are block offsets, as in the manual). */
    uint8_t *k = image + PRODOS_VDIR_KEY * PRODOS_BLOCK;
    const size_t len = strlen(name) > 15u ? 15u : strlen(name);
    k[0x04] = (uint8_t)(0xF0u | len);          /* storage type $F: volume header */
    memcpy(k + 0x05, name, len);               /* $05-$13 name, $14-$1B reserved */
    /* $1C-$1F creation date/time: 0 */
    k[0x20] = 0x00;                            /* version */
    k[0x21] = 0x00;                            /* min_version */
    k[0x22] = 0xC3;                            /* access: destroy, rename, write, read */
    k[0x23] = 0x27;                            /* entry_length */
    k[0x24] = 0x0D;                            /* entries_per_block */
    prodos_put16(k + 0x25, 0);                 /* file_count */
    prodos_put16(k + 0x27, PRODOS_BITMAP);     /* bit_map_pointer */
    prodos_put16(k + 0x29, blocks);            /* total_blocks */

    /* Bitmap: blocks 0..PRODOS_BITMAP used, the others free. */
    uint8_t *bm = image + PRODOS_BITMAP * PRODOS_BLOCK;
    for (uint32_t b = PRODOS_BITMAP + 1u; b < blocks; b++)
        bm[b >> 3] |= (uint8_t)(0x80u >> (b & 7u));
}

#endif /* PRODOS_FORMAT_H */
