;
; tk_io.s - I/O and timing primitives for the compiler test suite.
;
; Calling convention: parameters are on the software stack, first one at
; (sp)+0. 16-bit results are returned in X (low) / A (high).
;

#define ROM_PRINTCHAR $F5C1     ; Atmos 1.1 ROM: send character to printer
#define ROM_TIMER     $0272     ; 100Hz countdown timer, decremented by ROM IRQ


; void tk_putc(int c) - send a character to the emulated printer.
_tk_putc
	ldy #0
	lda (sp),y
	jsr ROM_PRINTCHAR
	rts


; void tk_timer_reset(void) - arm the 100Hz countdown timer at $FFFF.
_tk_timer_reset
	php
	sei
	lda #$FF
	sta ROM_TIMER
	sta ROM_TIMER+1
	plp
	rts


; unsigned int tk_timer_read(void) - elapsed 100Hz ticks since reset
; ($FFFF minus the current countdown value).
_tk_timer_read
	php
	sei
	sec
	lda #$FF
	sbc ROM_TIMER
	tax
	lda #$FF
	sbc ROM_TIMER+1
	plp
	rts
