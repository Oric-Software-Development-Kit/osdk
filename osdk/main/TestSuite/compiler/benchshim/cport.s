;
; Character output for the ISS benchmark samples: a single absolute store,
; the same cost shape as the benchmark's mos6502vm CPORT ($FFFF poke).
; On the Oric $FFFF is ROM space, so the store is a hardware no-op; a
; cport-aware emulator build can intercept it to capture the output.
;

__putc
	ldy #0
	lda (sp),y
	sta $FFFF
	rts

__puts
	ldy #0
	lda (sp),y
	sta tmp
	iny
	lda (sp),y
	sta tmp+1
	ldy #0
puts_loop
	lda (tmp),y
	beq puts_done
	sta $FFFF
	iny
	bne puts_loop
	inc tmp+1
	jmp puts_loop
puts_done
	rts
