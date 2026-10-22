; vtfont.s - the font of vtext.c: 128 glyphs, 8x8 for the 40x30 and 80x60
; modes (VT_MODE 4, 6), 8x16 for 80x30 (VT_MODE 8).

    .export _vt_font

    .segment "RODATA"

_vt_font:
.if VT_MODE = 8
    .incbin "font8x16.bin"
.else
    .incbin "font8x8.bin"
.endif
