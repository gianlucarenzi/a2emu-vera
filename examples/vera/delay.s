; delay.s -- busy waits counted in CPU cycles (the Apple II has no clock to read).
;
; void __fastcall__ delay_ms(unsigned int ms)
;   1 ms = 1,020.5 cycles at the Apple's 1.0205 MHz: 1,020 per iteration here
;   (+3 on the iterations where the low byte wraps, + the call).  Time spent
;   in interrupt handlers comes on top.

    .export _delay_ms

    .segment "CODE"

_delay_ms:
    sta lo
    stx hi
    ora hi
    beq done
loop:                       ; cycles
    ldx #199                ;    2
inner:
    dex                     ;  199 x 5 - 1 = 994
    bne inner
    lda lo                  ;    4
    bne nolo                ;    3 (2 + 6 more when lo = 0)
    dec hi
nolo:
    dec lo                  ;    6
    lda lo                  ;    4
    ora hi                  ;    4
    bne loop                ;    3   = 1,020
done:
    rts

    .segment "BSS"
lo: .res 1
hi: .res 1
