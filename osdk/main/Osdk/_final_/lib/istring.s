; String routines #1

;
; isalpha(c)
;
; @function _isalpha
_isalpha
	tax
	lda ctype,x
	and #$03	;_U | _L
	beq isalpha1
	jmp true
isalpha1
	jmp false
; @endfunction

;
; isupper(c)
;
; @function _isupper
_isupper
	tax
	lda ctype,x
	and #$01	;_U
	beq isupper1
	jmp true
isupper1
	jmp false
; @endfunction



;
; islower(c)
;
; @function _islower
_islower
	tax
	lda ctype,x
	and #$02	;_L
	beq islower1
	jmp true
islower1
	jmp false
; @endfunction
;
; isdigit(c)
;
; @function _isdigit
_isdigit
	tax
	lda ctype,x
	and #$04	;_N
	beq isdigit1
	jmp true
isdigit1
	jmp false
; @endfunction
;
; isxdigit(c)
;
; @function _isxdigi
_isxdigi
	tax
	lda ctype,x
	and #$44	;_N | _X
	beq isxdigit1
	jmp true
isxdigit1
	jmp false
; @endfunction
;
; isspace(c)
;
; @function _isspace
_isspace
	tax
	lda ctype,x
	and #$08	;_S
	beq isspace1
	jmp true
isspace1
	jmp false
; @endfunction
;
; ispunct(c)
;
; @function _ispunct
_ispunct
	tax
	lda ctype,x
	and #$10	;_P
	beq ispunct1
	jmp true
ispunct1
	jmp false
; @endfunction
;
; isalnum(c)
;
; @function _isalnum
_isalnum
	tax
	lda ctype,x
	and #$07	;_U | _L | _N
	beq isalnum1
	jmp true
isalnum1
	jmp false
; @endfunction
;
; isprint(c)
;
; @function _isprint
_isprint
	tax
	lda ctype,x
	and #$17	;_P | _U | _L | _N
	beq isprint1
	jmp true
isprint1
	jmp false
; @endfunction
;
; iscntrl(c)
;
; @function _iscntrl
_iscntrl
	tax
	lda ctype,x
	and #$20	;_C
	beq iscntrl1
	jmp true
iscntrl1
	jmp false
; @endfunction
;
; isascii(c)
;
; @function _isascii
_isascii
	and #$80	;0 if <= 127
	eor #$80	;invert
	beq isascii1
	jmp true
isascii1
	jmp false
; @endfunction
