; @function _getchar
;
; input char from keyboard
;
_getchar
	jsr $023B
	bpl _getchar	; loop until char available
	tax
	jsr $0238	; echo char
	lda #0
	rts
; @endfunction

; @function _putchar
;
; putchar(c) - also the conio _putc(c): identical (both output one char with
; the same LF handling), so __putc is a zero-cost alias here rather than a
; duplicate arg-load + jmp in conio.s.
;
__putc
_putchar
	ldy #0
	lda (sp),y
putchar
    cmp #$0A
    bne putchar2
    pha
    ldx #$0D
    jsr $0238
    pla
putchar2
    tax
	jmp $0238
; @endfunction


; @function _puts
;
; puts(char *string)
;
_puts
	ldy #0
	lda (sp),y
	sta tmp
	iny
	lda (sp),y
	sta tmp+1
	ldy #0
putsloop
	lda (tmp),y
	beq endputs
	jsr putchar
	iny
	bne putsloop
	inc tmp+1
	jmp putsloop
endputs
	lda #$0A
	jmp putchar
; @endfunction

; @function _gets
;
; gets(char buf[])
;
_gets
	ldy #0
	lda (sp),y
	sta tmp
	iny
	lda (sp),y
	sta tmp+1
gets
	ldy #0

getsloop
	jsr $023B
	cmp #$0D
	beq endgets
	cmp #$20
	bcc getsloop
	cmp #$7f
	beq backspace
	cpy #$ff
	beq getsloop
	sta (tmp),y
	iny
echochar
	tax
	jsr $0238
	jmp getsloop
backspace
	cpy #0
	beq getsloop
	dey
	jmp echochar
endgets
        lda #$0A
        jsr putchar
	lda #0
	sta (tmp),y
	ldx tmp
	lda tmp+1
	rts
; @endfunction
