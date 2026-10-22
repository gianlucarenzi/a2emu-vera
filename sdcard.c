/* sdcard.c -- an SD card in SPI mode, as the VERA's SPI port sees it.
 *
 * SDHC (block addressed, CCS = 1), 512-byte blocks, backed by a raw image file
 * (written through on every block).  Modelled from the SD Physical Layer spec,
 * SPI mode: every response comes one byte after the command (Ncr = 1) and every
 * data block one byte after its R1 (Nac = 1), so a host must poll for them as on
 * a real card.  Command CRCs are not checked (CRC is off in SPI mode); data
 * blocks carry a real CRC-16.
 *
 * Commands: CMD0 CMD1 CMD8 CMD9 CMD10 CMD12 CMD13 CMD16 CMD17 CMD18 CMD24 CMD25
 * CMD55 CMD58 CMD59, ACMD13 ACMD23 ACMD41 ACMD51.  Writes answer with the data
 * response token ($05 accepted, $0D on a read-only image) and then a few busy
 * bytes ($00); CMD25 takes $FC blocks until the $FD stop token.
 */
#include <stdio.h>
#include <string.h>
#include "sdcard.h"

#define BUSY_BYTES 8                   /* $00s after a block write / CMD12 */

static FILE    *img;
static uint32_t nblocks;
static int      readonly;
static int      selected;
static int      idle = 1;              /* until ACMD41 */
static int      acmd;                  /* the previous command was CMD55 */

static uint8_t  cmd[6];
static int      cmdlen;

static uint8_t  out[600];              /* bytes queued for MISO */
static int      outpos, outlen;

static enum { RX_CMD, RX_WTOKEN, RX_WDATA } rx = RX_CMD;
static int      wmulti;                /* the write is a CMD25 */
static uint32_t wlba;
static uint8_t  wbuf[514];             /* data + CRC */
static int      wlen;

static int      rmulti;                /* CMD18 streaming */
static uint32_t rlba;

static uint16_t crc16(const uint8_t *p, int n)
{
    uint16_t crc = 0;
    while (n--) {
        crc ^= (uint16_t)*p++ << 8;
        for (int b = 0; b < 8; b++)
            crc = crc & 0x8000 ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

static uint8_t crc7(const uint8_t *p, int n)
{
    uint8_t crc = 0;
    while (n--) {
        uint8_t d = *p++;
        for (int b = 0; b < 8; b++, d <<= 1) {
            crc <<= 1;
            if ((d ^ crc) & 0x80) crc ^= 0x09;
        }
    }
    return (uint8_t)((crc << 1) | 1);
}

static void push(uint8_t b)
{
    if (outlen < (int)sizeof out) out[outlen++] = b;
}

static void push_r1(uint8_t r1)
{
    outpos = outlen = 0;
    push(0xFF);                        /* Ncr */
    push(r1);
}

static void push_block(const uint8_t *data, int n)
{
    uint16_t crc = crc16(data, n);
    push(0xFF);                        /* Nac */
    push(0xFE);                        /* start block token */
    for (int i = 0; i < n; i++) push(data[i]);
    push(crc >> 8);
    push(crc & 0xFF);
}

static int read_lba(uint32_t lba, uint8_t *buf)
{
    if (lba >= nblocks || fseek(img, (long)lba * 512, SEEK_SET)) return 0;
    return fread(buf, 1, 512, img) == 512;
}

static void push_read(uint32_t lba)
{
    uint8_t buf[512];
    if (read_lba(lba, buf)) {
        push_block(buf, 512);
    } else {
        push(0xFF);
        push(0x08);                    /* data error token: out of range */
        rmulti = 0;
    }
}

static void push_csd(void)
{
    uint32_t c_size = nblocks / 1024 - 1;     /* in 512 KB units */
    uint8_t csd[16] = {
        0x40, 0x0E, 0x00, 0x32, 0x5B, 0x59, 0x00,
        (uint8_t)((c_size >> 16) & 0x3F), (uint8_t)(c_size >> 8), (uint8_t)c_size,
        0x7F, 0x80, 0x0A, 0x40, 0x00, 0x00 };
    csd[15] = crc7(csd, 15);
    push_block(csd, 16);
}

static void push_cid(void)
{
    uint8_t cid[16] = { 0x00, 'A', '2', 'V', 'E', 'R', 'A', ' ', 0x10,
                        0x12, 0x34, 0x56, 0x78, 0x01, 0x9A, 0x00 };
    cid[15] = crc7(cid, 15);
    push_block(cid, 16);
}

static void command(void)
{
    uint8_t  idx = cmd[0] & 0x3F;
    uint32_t arg = (uint32_t)cmd[1] << 24 | (uint32_t)cmd[2] << 16 | (uint32_t)cmd[3] << 8 | cmd[4];
    uint8_t  r1  = idle ? 0x01 : 0x00;
    int      was_acmd = acmd;

    acmd = 0;
    if (was_acmd) {
        switch (idx) {
        case 41: idle = 0; push_r1(0x00); return;                      /* SD_SEND_OP_COND */
        case 23: push_r1(r1); return;                                  /* SET_WR_BLK_ERASE_COUNT */
        case 13: push_r1(r1); push(0x00);                              /* SD_STATUS: R2 + 64 bytes */
                 { uint8_t st[64] = { 0 }; push_block(st, 64); } return;
        case 51: push_r1(r1);                                          /* SEND_SCR */
                 { uint8_t scr[8] = { 0x02, 0x35, 0x80, 0, 0, 0, 0, 0 }; push_block(scr, 8); } return;
        }                                                              /* else: as a plain command */
    }
    switch (idx) {
    case 0:  idle = 1; rmulti = 0; push_r1(0x01); break;               /* GO_IDLE_STATE */
    case 1:  idle = 0; push_r1(0x00); break;                           /* SEND_OP_COND (MMC) */
    case 8:  push_r1(r1); push(0x00); push(0x00);                      /* SEND_IF_COND: R7 */
             push(cmd[3] & 0x0F); push(cmd[4]); break;
    case 9:  push_r1(r1); push_csd(); break;
    case 10: push_r1(r1); push_cid(); break;
    case 12: rmulti = 0; outpos = outlen = 0;                          /* STOP_TRANSMISSION: R1b */
             push(0xFF); push(r1);
             for (int i = 0; i < BUSY_BYTES; i++) push(0x00);
             break;
    case 13: push_r1(r1); push(0x00); break;                           /* SEND_STATUS: R2 */
    case 16: push_r1(arg == 512 ? r1 : (uint8_t)(r1 | 0x40)); break;   /* SET_BLOCKLEN */
    case 17:                                                           /* READ_SINGLE_BLOCK */
    case 18:                                                           /* READ_MULTIPLE_BLOCK */
        if (arg >= nblocks) { push_r1(r1 | 0x20); break; }             /* address error */
        push_r1(r1);
        push_read(arg);
        rmulti = idx == 18;
        rlba = arg + 1;
        break;
    case 24:                                                           /* WRITE_BLOCK */
    case 25:                                                           /* WRITE_MULTIPLE_BLOCK */
        if (arg >= nblocks) { push_r1(r1 | 0x20); break; }
        push_r1(r1);
        rx = RX_WTOKEN;
        wmulti = idx == 25;
        wlba = arg;
        break;
    case 55: acmd = 1; push_r1(r1); break;                             /* APP_CMD */
    case 58: push_r1(r1);                                              /* READ_OCR: R3 */
             push(idle ? 0x40 : 0xC0); push(0xFF); push(0x80); push(0x00); break;
    case 59: push_r1(r1); break;                                       /* CRC_ON_OFF */
    default: push_r1(r1 | 0x04); break;                                /* illegal command */
    }
}

static void write_block(void)
{
    uint8_t token = 0x05;                                              /* data accepted */
    if (readonly || wlba >= nblocks || fseek(img, (long)wlba * 512, SEEK_SET)
        || fwrite(wbuf, 1, 512, img) != 512) {
        token = 0x0D;                                                  /* write error */
    } else {
        fflush(img);
    }
    outpos = outlen = 0;
    push(token);
    for (int i = 0; i < BUSY_BYTES; i++) push(0x00);
    wlba++;
}

static void receive(uint8_t in)
{
    switch (rx) {
    case RX_WTOKEN:
        if (in == 0xFE || (wmulti && in == 0xFC)) {
            rx = RX_WDATA;
            wlen = 0;
        } else if (wmulti && in == 0xFD) {                             /* stop tran token */
            rx = RX_CMD;
            outpos = outlen = 0;
            push(0xFF);
            for (int i = 0; i < BUSY_BYTES; i++) push(0x00);
        }
        return;
    case RX_WDATA:
        wbuf[wlen++] = in;
        if (wlen == (int)sizeof wbuf) {
            write_block();
            rx = wmulti ? RX_WTOKEN : RX_CMD;
        }
        return;
    case RX_CMD:
        if (cmdlen == 0 && (in & 0xC0) != 0x40) return;                /* not a command start */
        cmd[cmdlen++] = in;
        if (cmdlen == 6) {
            cmdlen = 0;
            command();
        }
        return;
    }
}

uint8_t sdcard_xfer(uint8_t in)
{
    uint8_t o = 0xFF;

    if (!img || !selected) return 0xFF;
    if (outpos < outlen) {
        o = out[outpos++];
        if (outpos == outlen) {
            outpos = outlen = 0;
            if (rmulti) push_read(rlba++);                             /* CMD18: next block */
        }
    }
    receive(in);
    return o;
}

void sdcard_select(int sel)
{
    if (sel && !selected) cmdlen = 0;  /* a new transaction starts with a new command */
    selected = sel;
}

int sdcard_attach(const char *path)
{
    long size;

    sdcard_detach();
    readonly = 0;
    if (!(img = fopen(path, "r+b"))) {
        if (!(img = fopen(path, "rb"))) return 0;
        readonly = 1;
    }
    if (fseek(img, 0, SEEK_END) || (size = ftell(img)) < 512 * 1024) {
        fclose(img);
        img = NULL;
        return 0;                       /* too small for an SDHC CSD */
    }
    nblocks = (uint32_t)(size / 512);
    idle = 1;
    acmd = cmdlen = outpos = outlen = rmulti = 0;
    rx = RX_CMD;
    return 1;
}

void sdcard_detach(void)
{
    if (img) fclose(img);
    img = NULL;
}

int sdcard_present(void)
{
    return img != NULL;
}
