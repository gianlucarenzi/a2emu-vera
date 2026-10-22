/* sdtest.c -- the SD card on the VERA's SPI port, driven as a real driver would:
 * init at the slow clock (CMD0, CMD8, ACMD41, CMD58), CSD, then single and
 * multiple block reads (CMD17, CMD18 + CMD12) and writes (CMD24, CMD25), with
 * the data CRC-16 checked.  The host checks the image afterwards (sdcheck.py). */
#include <conio.h>
#include <string.h>

#define SPI_DATA (*(volatile unsigned char *)0xC41E)   /* VERA in slot 4 */
#define SPI_CTRL (*(volatile unsigned char *)0xC41F)
#define SELECT   0x01
#define SLOW     0x02

static unsigned char buf[512], back[512];
static unsigned char ctrl, failed;

static unsigned char xfer(unsigned char b)
{
    SPI_DATA = b;
    while (SPI_CTRL & 0x80)
        ;
    return SPI_DATA;
}

static unsigned crc16(const unsigned char *p, unsigned n)
{
    unsigned crc = 0;
    unsigned char x;

    while (n--) {
        x = (crc >> 8) ^ *p++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((unsigned)x << 12) ^ ((unsigned)x << 5) ^ x;
    }
    return crc;
}

static unsigned char cmd(unsigned char idx, unsigned long arg, unsigned char crc)
{
    unsigned char r, i;

    xfer(0xFF);
    xfer(0x40 | idx);
    xfer(arg >> 24);
    xfer(arg >> 16);
    xfer(arg >> 8);
    xfer(arg);
    xfer(crc);
    for (i = 0; i < 10; ++i)               /* Ncr */
        if (!((r = xfer(0xFF)) & 0x80))
            return r;
    return 0xFF;
}

static void busy_wait(void)
{
    while (xfer(0xFF) != 0xFF)
        ;
}

static unsigned char read_data(unsigned char *p, unsigned n)   /* 0 = ok */
{
    unsigned i, crc;

    for (i = 0; i < 1000 && xfer(0xFF) != 0xFE; ++i)          /* Nac, start token */
        ;
    if (i == 1000) return 1;
    for (i = 0; i < n; ++i) p[i] = xfer(0xFF);
    crc = xfer(0xFF) << 8;
    crc |= xfer(0xFF);
    return crc != crc16(p, n);
}

static unsigned char write_data(unsigned char token, const unsigned char *p)
{
    unsigned i, crc = crc16(p, 512);
    unsigned char r;

    xfer(0xFF);
    xfer(token);
    for (i = 0; i < 512; ++i) xfer(p[i]);
    xfer(crc >> 8);
    xfer(crc);
    r = xfer(0xFF) & 0x1F;                 /* data response */
    busy_wait();
    return r != 0x05;
}

static void fill(unsigned char *p, unsigned long lba)
{
    unsigned i;
    for (i = 0; i < 512; ++i) p[i] = (unsigned char)(i * 13 + lba * 7) ^ 0x5A;
}

static void result(const char *what, unsigned char ok)
{
    cprintf("%-26s %s\r\n", what, ok ? "OK" : "ERROR");
    if (!ok) failed = 1;
}

int main(void)
{
    unsigned char r, i, ocr[4];
    unsigned long blocks;

    clrscr();
    cputs("VERA SPI / SD card test\r\n\r\n");

    SPI_CTRL = SLOW;                       /* deselected, <= 400 kHz */
    for (i = 0; i < 10; ++i) xfer(0xFF);   /* >= 74 clocks */
    ctrl = SLOW | SELECT;
    SPI_CTRL = ctrl;

    result("CMD0 go idle", cmd(0, 0, 0x95) == 0x01);
    r = cmd(8, 0x1AA, 0x87);
    for (i = 0; i < 4; ++i) ocr[i] = xfer(0xFF);
    result("CMD8 v2 card", r == 0x01 && ocr[2] == 0x01 && ocr[3] == 0xAA);
    for (i = 0; i < 100; ++i) {
        cmd(55, 0, 0x65);
        if (cmd(41, 0x40000000, 0x77) == 0) break;
    }
    result("ACMD41 ready", i < 100);
    r = cmd(58, 0, 0xFD);
    for (i = 0; i < 4; ++i) ocr[i] = xfer(0xFF);
    result("CMD58 SDHC (CCS)", r == 0 && (ocr[0] & 0xC0) == 0xC0);

    ctrl = SELECT;                         /* full speed from here on */
    SPI_CTRL = ctrl;

    r = cmd(9, 0, 0xFF);
    result("CMD9 CSD read", r == 0 && read_data(buf, 16) == 0);
    blocks = ((((unsigned long)(buf[7] & 0x3F) << 16) | (buf[8] << 8) | buf[9]) + 1) << 10;
    cprintf("  capacity %lu blocks\r\n", blocks);

    fill(back, 0);
    result("CMD17 read block 0", cmd(17, 0, 0xFF) == 0 && read_data(buf, 512) == 0
                                 && memcmp(buf, back, 512) == 0);

    r = cmd(18, 2, 0xFF) == 0;
    for (i = 2; r && i < 4; ++i) {
        fill(back, i);
        r = read_data(buf, 512) == 0 && memcmp(buf, back, 512) == 0;
    }
    r = r && cmd(12, 0, 0xFF) == 0;
    busy_wait();
    result("CMD18+12 read blocks 2-3", r);

    fill(buf, 1001);
    r = cmd(24, 1, 0xFF) == 0 && write_data(0xFE, buf) == 0;
    result("CMD24 write block 1", r);
    result("  reread block 1", cmd(17, 1, 0xFF) == 0 && read_data(back, 512) == 0
                               && memcmp(buf, back, 512) == 0);

    r = cmd(25, 4, 0xFF) == 0;
    for (i = 4; r && i < 6; ++i) {
        fill(buf, 1000 + i);
        r = write_data(0xFC, buf) == 0;
    }
    xfer(0xFF);
    xfer(0xFD);                            /* stop tran token */
    busy_wait();
    result("CMD25 write blocks 4-5", r);
    r = cmd(18, 4, 0xFF) == 0;
    for (i = 4; r && i < 6; ++i) {
        fill(back, 1000 + i);
        r = read_data(buf, 512) == 0 && memcmp(buf, back, 512) == 0;
    }
    r = r && cmd(12, 0, 0xFF) == 0;
    busy_wait();
    result("  reread blocks 4-5", r);

    SPI_CTRL = 0;                          /* deselect */
    cputs(failed ? "\r\nTEST FAILED\r\n" : "\r\nTEST PASSED\r\n");
    for (;;)
        ;
}
