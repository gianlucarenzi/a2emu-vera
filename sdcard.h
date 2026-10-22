/* sdcard.h -- an SD card (SDHC, SPI mode) behind the VERA's SPI port, backed by a
 * raw image file of 512-byte sectors. */
#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>

int     sdcard_attach(const char *path);   /* 1 = ok; read-only if the file is */
void    sdcard_detach(void);
int     sdcard_present(void);
void    sdcard_select(int selected);        /* chip select (active) */
/* one byte exchanged on the bus: `in` is what the host sent (MOSI), the result is
 * what the card drove on MISO during the same 8 clocks */
uint8_t sdcard_xfer(uint8_t in);

#endif
