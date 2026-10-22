; dblres.s -- IIe double-res video test: 16 colour bars (gendata.py).
; Needs aux memory (A2_AUX=64 or more).  A key steps through:
;   1. double hi-res   2. double lo-res   3. DHIRES off: plain hi-res of main
; Aux is written with 80STORE + PAGE2 (aux $0400-$07FF, and $2000-$3FFF with HIRES).
        .setcpu "6502"
        .org $0900
PTR     = $06

start:  sta $C00D               ; 80COL
        sta $C001               ; 80STORE
        sta $C057               ; HIRES: PAGE2 also maps aux $2000-$3FFF

        ; --- double hi-res: every line = dh_aux / dh_main
        sta $C055               ; PAGE2 -> aux
        ldx #<dh_aux
        ldy #>dh_aux
        jsr fillhgr
        sta $C054               ; main
        ldx #<dh_main
        ldy #>dh_main
        jsr fillhgr
        sta $C050               ; graphics
        sta $C052               ; full screen
        sta $C05E               ; DHIRES
        jsr waitkey

        ; --- double lo-res: every row = dl_aux / dl_main
        sta $C056               ; lo-res (PAGE2 maps aux $0400-$07FF)
        sta $C055
        ldx #<dl_aux
        ldy #>dl_aux
        jsr fillgr
        sta $C054
        ldx #<dl_main
        ldy #>dl_main
        jsr fillgr
        jsr waitkey

        ; --- DHIRES off: the main half of the DHGR bars as plain hi-res
        sta $C057
        sta $C05F
        jsr waitkey
        jmp start

waitkey:
:       lda $C000
        bpl :-
        sta $C010
        rts

; fillhgr: copy the 40 bytes at X/Y (lo/hi) to every hi-res line
fillhgr:
        stx src+1
        sty src+2
        ldx #0
hline:  lda hgr_lo,x
        sta PTR
        lda hgr_hi,x
        sta PTR+1
        jsr copy40
        inx
        cpx #192
        bne hline
        rts

; fillgr: the same for the 24 text/lo-res rows
fillgr: stx src+1
        sty src+2
        ldx #0
gline:  lda gr_lo,x
        sta PTR
        lda gr_hi,x
        sta PTR+1
        jsr copy40
        inx
        cpx #24
        bne gline
        rts

copy40: ldy #39
src:    lda $FFFF,y             ; patched
        sta (PTR),y             ; RAM, not a soft switch: the NMOS dummy read is harmless
        dey
        bpl src
        rts

        .include "data.inc"
