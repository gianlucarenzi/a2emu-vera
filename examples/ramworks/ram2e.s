; ram2e.s -- RAM2E II configuration commands test (a2run --aux RAM2E).
; The routines follow GWRAM.SYSTEM's ram2e_hal.c (Garrett's Workshop).
; PHASE ($0300, loaded by the Makefile) picks the run:
;   0: fresh card (no saved settings): detection, LED, capacity mask, the
;      command timeout; then saves mask $0F + LED on in the settings memory
;   1: next power-on with that file: 16 banks, LED on; then erases the settings
;   2: next power-on: defaults again (128 banks, LED off)
; Exit code (debug card $C0B1): 100 = passed, otherwise the number of the check
; that failed.
        .setcpu "6502"
        .org $0800
EXIT    = $C0B1
RWBANK  = $C073
PHASE   = $0300
RES     = $0301
CMD     = $0302
ARG     = $0303

start:  lda #0
        sta RWBANK
        sta $C008               ; ALTZP off
        lda PHASE
        beq phase0
        cmp #1
        bne :+
        jmp phase1
:       jmp phase2

; ---------------------------------------------------------------- phase 0
phase0: lda #$FF                ; 1: MAX chip detect -> bank $FF
        jsr detect
        ldx #1
        jsr expect1
        lda #$FD                ; 2: not a MachXO2
        jsr detect
        ldx #2
        jsr expect0
        lda #$F0                ; 3: LED present
        jsr detect
        ldx #3
        jsr expect1
        lda #$F1                ; 4: rev. C
        jsr detect
        ldx #4
        jsr expect1
        jsr getsize             ; 5: default mask $7F = 128 banks
        ldx #5
        cmp #128
        jsr expeq
        lda #$E3                ; 6: LED off by default (erased settings)
        jsr detect
        ldx #6
        jsr expect0
        lda #$E2                ; 7: LED on
        ldy #1
        jsr ram2e_cmd
        lda #$E3
        jsr detect
        ldx #7
        jsr expect1
        lda #$E0                ; 8: mask $07 = 512 KB
        ldy #$07
        jsr ram2e_cmd
        jsr getsize
        ldx #8
        cmp #8
        jsr expeq
        lda #$E0                ; 9: mask $00 = 64 KB
        ldy #$00
        jsr ram2e_cmd
        jsr getsize
        ldx #9
        cmp #1
        jsr expeq
        lda #$E0                ; 10: mask $FF: 128 banks, no 16 MB
        ldy #$FF
        jsr ram2e_cmd
        jsr getsize
        ldx #10
        cmp #128
        jsr expeq
        lda #$E0                ; 11: 7 idle cycles between two writes: still a command
        ldy #$03
        jsr cmd_slow1
        jsr getsize
        ldx #11
        cmp #4
        jsr expeq
        lda #$E0                ; 12: 9 idle cycles: timed out, mask stays $03
        ldy #$FF
        jsr cmd_slow2
        jsr getsize
        ldx #12
        cmp #4
        jsr expeq
        lda #$E0                ; 13: plain RamWorks bank writes stay masked
        ldy #$0F
        jsr ram2e_cmd
        jsr getsize
        ldx #13
        cmp #16
        jsr expeq

        ; save mask $0F + LED on (ram2e_max_save): 8 mask bits MSB first, the
        ; LED bit (0 if LED == mask bit 7), seven 1s; then "program"
        lda #$0F
        sta RES
        ldx #8
savel:  txa
        pha
        lda #$80
        asl RES
        bcc :+
        lda #$C0
:       tay
        lda #$EA
        jsr ram2e_cmd
        pla
        tax
        dex
        bne savel
        ldx #8                  ; LED bit 1, then seven 1s
saveo:  txa
        pha
        lda #$EA
        ldy #$C0
        jsr ram2e_cmd
        pla
        tax
        dex
        bne saveo
        lda #$EF
        ldy #0
        jsr ram2e_cmd
        jmp pass

; ---------------------------------------------------------------- phase 1
phase1: jsr getsize             ; 21: saved mask $0F
        ldx #21
        cmp #16
        jsr expeq
        lda #$E3                ; 22: saved LED on
        jsr detect
        ldx #22
        jsr expect1
        lda #$EE                ; erase the settings
        ldy #0
        jsr ram2e_cmd
        jmp pass

; ---------------------------------------------------------------- phase 2
phase2: jsr getsize             ; 31: erased: default 128 banks
        ldx #31
        cmp #128
        jsr expeq
        lda #$E3                ; 32: LED off
        jsr detect
        ldx #32
        jsr expect0
        jmp pass

; ---------------------------------------------------------------- checks
expect1: lda RES                ; RES must be 1, else exit X
        cmp #1
expeq:  beq ok                  ; Z set = pass, else exit X
fail:   stx EXIT
        jmp *
expect0: lda RES
        bne fail
ok:     rts
pass:   lda #100
        sta EXIT
        jmp *

; ---------------------------------------------------------------- RAM2E
; ram2e_cmd: command A, argument Y
ram2e_cmd:
        tax
        lda #0                  ; reset any half-done sequence
        sta RWBANK
        sta RWBANK
        lda #$FF
        sta RWBANK
        lda #$00
        sta RWBANK
        lda #$55
        sta RWBANK
        lda #$AA
        sta RWBANK
        lda #$C1
        sta RWBANK
        lda #$AD
        sta RWBANK
        stx RWBANK
        sty RWBANK
        lda #0
        sta RWBANK
        rts

; the same with NOPS extra NOPs between the $AA and $C1 writes: write cycles
; 4 + 2 (+ 2 per NOP) apart, so 1 NOP = 7 idle cycles, 2 NOPs = 9
.macro  CMD_SLOW nops
        sta CMD
        sty ARG
        lda #0
        sta RWBANK
        sta RWBANK
        lda #$FF
        sta RWBANK
        lda #$00
        sta RWBANK
        lda #$55
        sta RWBANK
        lda #$AA
        sta RWBANK
        lda #$C1
        .repeat nops
        nop
        .endrepeat
        sta RWBANK
        lda #$AD
        sta RWBANK
        lda CMD
        sta RWBANK
        lda ARG
        sta RWBANK
        lda #0
        sta RWBANK
        rts
.endmacro
cmd_slow1: CMD_SLOW 1
cmd_slow2: CMD_SLOW 2

; detect: send command A (a SetRWBankFF-class one); RES = 1 if the bank
; became $FF (ram2e_detect)
detect: tax
        sta $C009               ; ALTZP
        lda #$00
        sta RWBANK
        sta $00
        lda #$FF
        sta RWBANK
        lda #$00
        sta RWBANK
        lda #$55
        sta RWBANK
        lda #$AA
        sta RWBANK
        lda #$C1
        sta RWBANK
        lda #$AD
        sta RWBANK
        stx RWBANK
        lda #$00
        sta RWBANK
        lda #$FF                ; into bank $FF on a RAM2E, bank 0 otherwise
        sta $00
        lda #$00
        sta RWBANK
        sta RWBANK
        ldx $00                 ; bank 0 still 0 -> RAM2E answered
        sta $C008
        lda #0
        cpx #0
        bne :+
        lda #1
:       sta RES
        rts

; getsize: A = number of banks with their own data (ramworks_getsize)
getsize:
        sta $C009
        ldy #$FF
gs_set: sty RWBANK
        sty $00
        dey
        cpy #$FF
        bne gs_set
        ldy #0
        ldx #0
gs_cnt: sty RWBANK
        cpy $00
        bne :+
        inx
:       iny
        bne gs_cnt
        sta $C008
        lda #0
        sta RWBANK
        txa
        rts
