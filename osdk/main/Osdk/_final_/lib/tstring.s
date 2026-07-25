;
; toupper(c)
;
; @function _toupper
_toupper
	tax
_touppermc	
	lda ctype,x
	and #$02	;_L
	beq toupper1	;skip if not lower-case
	sec
	txa		;original char
	sbc #$20	;force upper case
	tax
toupper1
	lda #0
	rts
; @endfunction

;
; tolower(c)
;
; @function _tolower
_tolower
	tax
	lda ctype,x
	and #$01	;_U
	beq tolower1	;skip if not upper-case
	clc
	txa		;original char
	adc #$20	;force lower case
	tax
tolower1
	lda #0
	rts
; @endfunction

;
; toascii(c)
;
; @function _toascii
_toascii
	and #$7f
	tax
	lda #0
	rts
; @endfunction
	
