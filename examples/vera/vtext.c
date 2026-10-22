/*
 * vtext.c - a minimal text console on the VERA layer 1 (see vtext.h).
 */

#include "vera.h"
#include "vtext.h"

/* the 128 glyphs (vtfont.s) */
extern const unsigned char vt_font[];

unsigned char vt_top;               /* first tilemap row on the screen */
static unsigned char cur_x, cur_y;  /* cursor on the screen */

/* point DATA0 at column x of tilemap row r, increment 1 */
static void __fastcall__ set_cell(unsigned char x, unsigned char r)
{
    VERA_CTRL   = 0;
    VERA_ADDR_L = (unsigned char)(x << 1);
    VERA_ADDR_M = (unsigned char)(0xB0 + (r & (VT_MAP_ROWS - 1)));
    VERA_ADDR_H = VERA_INC1 | 1;
}

static void __fastcall__ clear_row(unsigned char r)
{
    unsigned char x;

    set_cell(0, r);
    for (x = 0; x < VT_COLS; ++x) {
        vera_w0(0x20);
        vera_w0(VT_COLOR);
    }
}

static void set_vscroll(void)
{
    unsigned int v = (unsigned int)vt_top * VT_TILE_HEIGHT;

    VERA_CTRL      = 0;
    VERA_L1_VSCR_L = (unsigned char)v;
    VERA_L1_VSCR_H = (unsigned char)(v >> 8);
}

void vt_clear(void)
{
    unsigned char r;

    for (r = 0; r < VT_MAP_ROWS; ++r)
        clear_row(r);
    vt_top = 0;
    cur_x = cur_y = 0;
    set_vscroll();
}

void vt_init(void)
{
    unsigned int i;

    VERA_CTRL      = 0;                         /* DCSEL 0 */
    VERA_DC_VIDEO  = 0x21;                      /* VGA, layer 1 only */
#if VT_MODE == 4
    VERA_DC_HSCALE = 0x40;
    VERA_DC_VSCALE = 0x40;
#else
    VERA_DC_HSCALE = 0x80;
    VERA_DC_VSCALE = 0x80;
#endif
    VERA_DC_BORDER = 0;
    VERA_L1_CONFIG   = 0x60;                    /* 128x64 tiles, 1 bpp text */
    VERA_L1_MAPBASE  = (unsigned char)(VT_MAP >> 9);
#if VT_TILE_HEIGHT == 16
    VERA_L1_TILEBASE = (unsigned char)(VT_FONT >> 9) | 2;  /* 8x16 tiles */
#else
    VERA_L1_TILEBASE = (unsigned char)(VT_FONT >> 9);      /* 8x8 tiles */
#endif
    VERA_L1_HSCR_L = 0;
    VERA_L1_HSCR_H = 0;

    VERA_SET_ADDR(VT_FONT, VERA_INC1);
    for (i = 0; i < 128u * VT_TILE_HEIGHT; ++i)
        vera_w0(vt_font[i]);

    vt_clear();
}

static void newline(void)
{
    cur_x = 0;
    if (cur_y < VT_ROWS - 1) {
        ++cur_y;
        return;
    }
    /* scroll: the row below the screen becomes the new last row */
    clear_row((unsigned char)(vt_top + VT_ROWS));
    vt_top = (unsigned char)((vt_top + 1) & (VT_MAP_ROWS - 1));
    set_vscroll();
}

void __fastcall__ vt_glyph(unsigned char g)
{
    set_cell(cur_x, (unsigned char)(vt_top + cur_y));
    vera_w0(g & 0x7F);
    vera_w0((g & 0x80) ? (unsigned char)((VT_COLOR << 4) | (VT_COLOR >> 4)) : VT_COLOR);
    if (++cur_x >= VT_COLS)
        newline();
}

void __fastcall__ vt_putc(char c)
{
    if (c == '\n') {
        newline();
        return;
    }
    vt_glyph((unsigned char)c);     /* cc65 apple2 strings are ASCII */
}

void __fastcall__ vt_puts(const char *s)
{
    while (*s)
        vt_putc(*s++);
}
