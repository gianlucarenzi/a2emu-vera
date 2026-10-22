; vera_io.s -- writes to the VERA registers that have read side effects.
;
; cc65 reaches the card through the `vera` pointer: a store is STA (zp),Y, and
; the NMOS 6502 (][+, unenhanced IIe) reads the target address one cycle before
; it writes it.  On DATA0/DATA1 that dummy read steps ADDR (or fills the FX
; cache), on SPI_DATA it starts an auto-tx transfer, on $09/$0A with DCSEL 6
; it runs the FX accumulator.  STA absolute does no dummy read: these routines
; store with absolute addresses whose high byte vera_io_init() patches to the
; card's page ($Cn) once vera_detect() has found it.

    .export _vera_w0, _vera_w1, _vera_poke, _vera_io_init

    .segment "CODE"

; void __fastcall__ vera_w0(unsigned char v): DATA0 = v
_vera_w0:
w0: sta $C403
    rts

; void __fastcall__ vera_w1(unsigned char v): DATA1 = v
_vera_w1:
w1: sta $C404
    rts

; void __fastcall__ vera_poke(unsigned int reg_val): register (high byte, X)
; = value (low byte, A); see VERA_POKE() in vera.h
_vera_poke:
    stx pk+1
pk: sta $C400
    rts

; void __fastcall__ vera_io_init(unsigned char page): page = $Cn
_vera_io_init:
    sta w0+2
    sta w1+2
    sta pk+2
    rts
