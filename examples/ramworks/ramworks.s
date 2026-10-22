; ramworks.s -- RamWorks III aux banking test (a2run --aux 64K..8M).
; Runs at $0400 with 80STORE on / PAGE2 off, so the code stays in main RAM
; whatever RAMRD says. Exit code (debug card $C0B1):
; number of aux banks that hold their own data (0..128), 200 = main RAM clobbered.
        .setcpu "6502"
        .org $0400
EXIT    = $C0B1
CNT     = $0700             ; scratch in main text page (80STORE, PAGE2 off)

start:  sta $C001           ; 80STORE on
        sta $C008           ; ALTZP off
        lda $C083
        lda $C083           ; LC bank 2 read/write
        lda #$A5
        sta $3000           ; main markers
        sta $F0
        sta $D000

        ldx #127            ; write phase: bank b gets b, descending
wloop:  stx $C073
        sta $C005           ; RAMWRT on
        stx $3000
        txa
        eor #$FF
        sta $3001
        sta $C004           ; RAMWRT off
        sta $C009           ; ALTZP on: aux ZP + aux LC
        stx $F0
        stx $D000
        sta $C008
        dex
        bpl wloop

        ldx #0              ; read phase: count banks until the first mismatch
rloop:  stx $C073
        sta $C003           ; RAMRD on
        cpx $3000
        bne done0
        txa
        eor #$FF
        cmp $3001
        bne done0
        sta $C002           ; RAMRD off
        sta $C009
        cpx $F0
        bne done1
        cpx $D000
        bne done1
        sta $C008
        inx
        bpl rloop
        jmp done
done0:  sta $C002
done1:  sta $C008
done:   stx CNT
        lda #0
        sta $C073           ; back to bank 0
        lda #$A5
        cmp $3000
        bne bad
        cmp $F0
        bne bad
        cmp $D000
        bne bad
        lda CNT
        sta EXIT
        jmp *
bad:    lda #200
        sta EXIT
        jmp *
