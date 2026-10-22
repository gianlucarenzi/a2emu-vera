; ============================================================================
; vera_irq.s - VERA IRQ hook on the Apple II (port of VERA_C64/tests/vera_irq.s)
;
; See vera_irq.h for the C interface.
;
; A cc65 program booted by sdk/boot.s runs with the language card RAM read
; enabled ($C080), so the 6502 takes its IRQ vector from $FFFE in LC RAM, not
; from the Monitor ROM (there is no DOS, no ROM IRQ path through $03FE).  The
; hook goes straight into that vector: written with $C081 $C081 (ROM read, LC
; RAM write), then $C080 again.  The handler saves A, X, Y itself and ends with
; RTI; an IRQ that is not the VERA's goes on to the previous vector (with the
; registers restored) if there was one, else it just returns.
;
; The VERA's address is known at run time only: the handler and the helpers
; use absolute addresses ($C4xx in the source) whose high byte
; vera_irq_install() patches to the page vera_detect() found.  ADDR, DATA and
; CTRL are never touched, so the hook can interrupt a VRAM access sequence.
; ============================================================================

    .setcpu "6502"

    .import _vera                       ; volatile unsigned char *vera

VEC             = $FFFE                 ; 6502 IRQ/BRK vector (LC RAM here)
LCROM_WRAM      = $C081                 ; twice: ROM read, LC RAM bank 2 write
LCRAM_RO        = $C080                 ; LC RAM bank 2 read, write protected
IRQ_MASK        = $0F                   ; IEN/ISR bits 3:0 = AFLOW|SPRCOL|LINE|VSYNC

    .export _vera_irq_install
    .export _vera_irq_remove
    .export _vera_irq_enable
    .export _vera_irq_disable
    .export _vera_irq_take
    .export _vera_irq_set_line
    .export _vera_irq_set_callback
    .export _vera_irq_count
    .export _vera_irq_vector

    .segment "BSS"

old_vec:        .res 2
callback:       .res 2
installed:      .res 1
flags:          .res 1
pending:        .res 1
tmp:            .res 1
tmp2:           .res 1
_vera_irq_count: .res 4

    .segment "CODE"

; every VERA register access: the high byte of its operand (site+2) is
; patched (the routines use plain labels: a macro ends the @local scopes)
.macro VREG op, reg, site
site = *
        op  $C400 + reg
.endmacro

; ----------------------------------------------------------------------------
; The IRQ handler
; ----------------------------------------------------------------------------
irq_handler:
    pha
    txa
    pha
    tya
    pha
    VREG lda, $07, p0           ; ISR
    VREG and, $06, p1           ; & IEN
    and #IRQ_MASK
    bne h_ours
    lda old_vec+1               ; not the VERA: the previous handler, if any
    beq h_exit
    pla
    tay
    pla
    tax
    pla
    jmp (old_vec)

h_ours:
    sta pending
    and #$07
    beq h_aflow
    VREG sta, $07, p2           ; ack VSYNC/LINE/SPRCOL (write 1 clears only these)
h_aflow:
    lda pending
    and #$08
    beq h_record
    VREG lda, $06, p3           ; AFLOW cannot be cleared: stop the level IRQ
    and #$F7
    VREG sta, $06, p4
h_record:
    lda pending
    ora flags
    sta flags

    lda pending                 ; per-source counters
    lsr a
    bcc h_c1
    inc _vera_irq_count+0
h_c1:
    lsr a
    bcc h_c2
    inc _vera_irq_count+1
h_c2:
    lsr a
    bcc h_c3
    inc _vera_irq_count+2
h_c3:
    lsr a
    bcc h_cb
    inc _vera_irq_count+3
h_cb:
    lda callback+1
    beq h_exit
    lda pending                 ; A = serviced sources
    jsr call_cb
h_exit:
    pla
    tay
    pla
    tax
    pla
    rti

call_cb:
    jmp (callback)

; ----------------------------------------------------------------------------
; unsigned int vera_irq_vector(void): the IRQ vector at $FFFE as the CPU sees it
; ----------------------------------------------------------------------------
_vera_irq_vector:
    lda VEC
    ldx VEC+1
    rts

; write A (low), X (high) into the IRQ vector in LC RAM; interrupts off
set_vec:
    bit LCROM_WRAM
    bit LCROM_WRAM
    sta VEC
    stx VEC+1
    bit LCRAM_RO
    rts

; ----------------------------------------------------------------------------
; void vera_irq_install(void): after vera_detect(); enables 6502 IRQs (CLI)
; ----------------------------------------------------------------------------
_vera_irq_install:
    lda installed
    bne install_done
    lda _vera+1                 ; patch the VERA page into every access
    sta p0+2
    sta p1+2
    sta p2+2
    sta p3+2
    sta p4+2
    sta p5+2
    sta p6+2
    sta p7+2
    sta p8+2
    sta p9+2
    sta p10+2
    sta p11+2
    sta p12+2
    sta p13+2
    sta p14+2
    sei
    lda VEC
    sta old_vec
    lda VEC+1
    sta old_vec+1
    lda #0
    sta flags
    sta callback
    sta callback+1
    ldx #3
install_clr:
    sta _vera_irq_count,x
    dex
    bpl install_clr
    lda #<irq_handler
    ldx #>irq_handler
    jsr set_vec
    lda #1
    sta installed
    cli
install_done:
    rts

; ----------------------------------------------------------------------------
; void vera_irq_remove(void)
; ----------------------------------------------------------------------------
_vera_irq_remove:
    lda installed
    beq remove_done
    php
    sei
    VREG lda, $06, p5           ; all four sources off, keep IRQ_LINE[8]
    and #$80
    VREG sta, $06, p6
    lda VEC                     ; restore only if the vector is still ours
    cmp #<irq_handler
    bne remove_keep
    lda VEC+1
    cmp #>irq_handler
    bne remove_keep
    lda old_vec
    ldx old_vec+1
    jsr set_vec
    lda #0
    sta installed
remove_keep:
    plp
remove_done:
    rts

; ----------------------------------------------------------------------------
; void __fastcall__ vera_irq_enable(unsigned char mask)    A = mask
; ----------------------------------------------------------------------------
_vera_irq_enable:
    and #IRQ_MASK
    sta tmp
    php
    sei
    lda tmp
    and #$07
    VREG sta, $07, p7           ; drop stale status of the sources being enabled
    VREG lda, $06, p8
    and #$8F                    ; keep IRQ_LINE[8] and the current enables
    ora tmp
    VREG sta, $06, p9
    plp
    rts

; ----------------------------------------------------------------------------
; void __fastcall__ vera_irq_disable(unsigned char mask)   A = mask
; ----------------------------------------------------------------------------
_vera_irq_disable:
    and #IRQ_MASK
    eor #$FF                    ; bits 7:4 stay 1: IRQ_LINE[8] preserved
    sta tmp
    php
    sei
    VREG lda, $06, p10
    and tmp
    VREG sta, $06, p11
    plp
    rts

; ----------------------------------------------------------------------------
; unsigned char vera_irq_take(void)   returns and clears the serviced flags
; ----------------------------------------------------------------------------
_vera_irq_take:
    php
    sei
    lda flags
    ldx #0
    stx flags
    plp
    rts

; ----------------------------------------------------------------------------
; void __fastcall__ vera_irq_set_line(unsigned int line)   A = low, X = high
; ----------------------------------------------------------------------------
_vera_irq_set_line:
    sta tmp
    txa
    and #$01
    sta tmp2
    php
    sei
    lda tmp
    VREG sta, $08, p12          ; IRQLINE_L
    VREG lda, $06, p13
    and #IRQ_MASK               ; current enables, IRQ_LINE[8] rebuilt below
    ldx tmp2
    beq set_line_b8
    ora #$80
set_line_b8:
    VREG sta, $06, p14
    plp
    rts

; ----------------------------------------------------------------------------
; void __fastcall__ vera_irq_set_callback(void (*cb)(void))  A = low, X = high
; ----------------------------------------------------------------------------
_vera_irq_set_callback:
    php
    sei
    sta callback
    stx callback+1
    plp
    rts
