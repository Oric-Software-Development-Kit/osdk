; int strlen(char *s)
_strlen
	sta tmp			; __fastcall: string pointer in A:X (low:high)
	stx tmp+1
	ldy #0
	ldx #0
	
looplen
	lda (tmp),y
	beq endstrlen
	iny
	bne looplen
	inc tmp+1
	inx
	bne looplen
endstrlen
	sty tmp
	txa
	ldx tmp
	rts
