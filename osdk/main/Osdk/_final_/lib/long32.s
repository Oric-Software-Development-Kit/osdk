;
; 32-bit (long) arithmetic runtime.
;
; Calling convention (see MACROS.H, the L macro families):
;   - operand A  : op1..op1+1 = low word, op2..op2+1 = high word
;                  (op1/op2 are 4 CONTIGUOUS zero-page bytes, so op1,x
;                  indexing walks the whole value, byte 0 = LSB)
;   - operand B  : pointed to by `tmp` (4 bytes, little endian);
;                  a constant operand is first materialized into
;                  `lscratch` by the LPTB_C macro helper
;   - result     : op1..op2+1 (the calling macro stores it per its
;                  addressing mode)
;   - clobbers   : A, X, Y; `tmp` itself is left pointing at B
;
; Nothing here touches the C temporaries (tmp0-7) or registers (reg0-7),
; so the compiler's live values survive these calls - same contract as
; mul16i/div16i.
;
	.text

; ---- add / subtract / bitwise: straight unrolled 4-byte chains --------

+ladd32
	ldy #0
	clc
	lda op1
	adc (tmp),y
	sta op1
	iny
	lda op1+1
	adc (tmp),y
	sta op1+1
	iny
	lda op2
	adc (tmp),y
	sta op2
	iny
	lda op2+1
	adc (tmp),y
	sta op2+1
	rts

+lsub32
	ldy #0
	sec
	lda op1
	sbc (tmp),y
	sta op1
	iny
	lda op1+1
	sbc (tmp),y
	sta op1+1
	iny
	lda op2
	sbc (tmp),y
	sta op2
	iny
	lda op2+1
	sbc (tmp),y
	sta op2+1
	rts

+land32
	ldy #0
	lda op1
	and (tmp),y
	sta op1
	iny
	lda op1+1
	and (tmp),y
	sta op1+1
	iny
	lda op2
	and (tmp),y
	sta op2
	iny
	lda op2+1
	and (tmp),y
	sta op2+1
	rts

+lor32
	ldy #0
	lda op1
	ora (tmp),y
	sta op1
	iny
	lda op1+1
	ora (tmp),y
	sta op1+1
	iny
	lda op2
	ora (tmp),y
	sta op2
	iny
	lda op2+1
	ora (tmp),y
	sta op2+1
	rts

+lxor32
	ldy #0
	lda op1
	eor (tmp),y
	sta op1
	iny
	lda op1+1
	eor (tmp),y
	sta op1+1
	iny
	lda op2
	eor (tmp),y
	sta op2
	iny
	lda op2+1
	eor (tmp),y
	sta op2+1
	rts

; ---- unary (operate on op1:op2 in place, no B operand) ----------------

+lcom32
	lda op1
	eor #$ff
	sta op1
	lda op1+1
	eor #$ff
	sta op1+1
	lda op2
	eor #$ff
	sta op2
	lda op2+1
	eor #$ff
	sta op2+1
	rts

+lneg32
	sec
	lda #0
	sbc op1
	sta op1
	lda #0
	sbc op1+1
	sta op1+1
	lda #0
	sbc op2
	sta op2
	lda #0
	sbc op2+1
	sta op2+1
	rts

; ---- multiply: 32x32 -> low 32 (shift-add; sign-agnostic) -------------

+lmul32
	; lwork = B (the shifting addend), lrem = product accumulator
	ldy #3
copyb32
	lda (tmp),y
	sta lwork,y
	lda #0
	sta lrem,y
	dey
	bpl copyb32
	ldx #32
mulloop32
	; shift A right, LSB into carry
	lsr op2+1
	ror op2
	ror op1+1
	ror op1
	bcc mulskip32
	; product += lwork
	clc
	lda lrem
	adc lwork
	sta lrem
	lda lrem+1
	adc lwork+1
	sta lrem+1
	lda lrem+2
	adc lwork+2
	sta lrem+2
	lda lrem+3
	adc lwork+3
	sta lrem+3
mulskip32
	; lwork <<= 1
	asl lwork
	rol lwork+1
	rol lwork+2
	rol lwork+3
	dex
	bne mulloop32
	; result -> op1:op2
	lda lrem
	sta op1
	lda lrem+1
	sta op1+1
	lda lrem+2
	sta op2
	lda lrem+3
	sta op2+1
	rts

; ---- unsigned divide / modulo: classic 32-bit restoring division ------
; quotient develops in op1:op2 (dividend shifts out as quotient shifts
; in), remainder accumulates in lrem. lwork holds the divisor copy.

+ldiv32u
	ldy #3
copyd32
	lda (tmp),y
	sta lwork,y
	lda #0
	sta lrem,y
	dey
	bpl copyd32
	ldx #32
divloop32
	; shift dividend/quotient left, MSB into remainder
	asl op1
	rol op1+1
	rol op2
	rol op2+1
	rol lrem
	rol lrem+1
	rol lrem+2
	rol lrem+3
	; if remainder >= divisor: remainder -= divisor, quotient |= 1
	sec
	lda lrem
	sbc lwork
	tay			; keep the difference bytes in Y/ldtmp until
	lda lrem+1		; we know whether the subtraction held
	sbc lwork+1
	sta ldtmp
	lda lrem+2
	sbc lwork+2
	sta ldtmp+1
	lda lrem+3
	sbc lwork+3
	bcc divskip32		; remainder < divisor: leave it alone
	sta lrem+3
	lda ldtmp+1
	sta lrem+2
	lda ldtmp
	sta lrem+1
	sty lrem
	inc op1			; low bit of the (just shifted) quotient is 0
divskip32
	dex
	bne divloop32
	rts

+lmod32u
	jsr ldiv32u
	lda lrem
	sta op1
	lda lrem+1
	sta op1+1
	lda lrem+2
	sta op2
	lda lrem+3
	sta op2+1
	rts

; ---- signed divide / modulo (C89: quotient truncates toward zero,
;      remainder takes the sign of the dividend) ------------------------
; B cannot be negated in place (it may be a user variable or constant),
; so the sign pass copies |B| into lscratch and repoints tmp at it.

lsignprep
	; lsign = dividend sign (bit7), lsign+1 = sign of the quotient
	lda op2+1
	sta lsign
	ldy #3
	lda (tmp),y
	eor op2+1
	sta lsign+1
	; A = |A|
	bit lsign
	bpl labspos
	jsr lneg32
labspos
	; B = |B| (copy into lscratch when negative, repoint tmp)
	ldy #3
	lda (tmp),y
	bpl lbabs_done
	; lscratch = 0 - B
	sec
	ldy #0
	lda #0
	sbc (tmp),y
	sta lscratch
	iny
	lda #0
	sbc (tmp),y
	sta lscratch+1
	iny
	lda #0
	sbc (tmp),y
	sta lscratch+2
	iny
	lda #0
	sbc (tmp),y
	sta lscratch+3
	lda #<lscratch
	sta tmp
	lda #>lscratch
	sta tmp+1
lbabs_done
	rts

+ldiv32i
	jsr lsignprep
	jsr ldiv32u
	bit lsign+1
	bpl ldivi_done
	jmp lneg32		; negative quotient
ldivi_done
	rts

+lmod32i
	jsr lsignprep
	jsr lmod32u
	bit lsign		; remainder follows the dividend's sign
	bpl lmodi_done
	jmp lneg32
lmodi_done
	rts

; ---- shifts: count in B's low byte (undefined beyond 31, like C) ------

+llsh32
	ldy #0
	lda (tmp),y
	tax
	beq lshdone32
lshloop32
	asl op1
	rol op1+1
	rol op2
	rol op2+1
	dex
	bne lshloop32
lshdone32
	rts

+lrshl32			; logical (unsigned) >>
	ldy #0
	lda (tmp),y
	tax
	beq rshldone32
rshlloop32
	lsr op2+1
	ror op2
	ror op1+1
	ror op1
	dex
	bne rshlloop32
rshldone32
	rts

+lasr32				; arithmetic (signed) >>
	ldy #0
	lda (tmp),y
	tax
	beq asrdone32
asrloop32
	lda op2+1
	cmp #$80		; carry = sign bit
	ror op2+1
	ror op2
	ror op1+1
	ror op1
	dex
	bne asrloop32
asrdone32
	rts

; ---- compares: return the relation in the C and Z flags ---------------
; lcmp32u: unsigned A vs B.  C=1 <=> A>=B,  Z=1 <=> A==B
;   (byte-wise from the most significant byte; the first difference
;   decides, equality falls through with Z set)

+lcmp32u
	ldy #3
	lda op2+1
	cmp (tmp),y
	bne lcmpdone
	dey
	lda op2
	cmp (tmp),y
	bne lcmpdone
	dey
	lda op1+1
	cmp (tmp),y
	bne lcmpdone
	dey
	lda op1
	cmp (tmp),y
lcmpdone
	rts

; lcmp32i: signed A vs B, same C/Z semantics (C=1 <=> A>=B signed).
; Signed compare == unsigned compare with both sign bits flipped.

+lcmp32i
	ldy #3
	lda (tmp),y
	eor #$80
	sta lsign
	lda op2+1
	eor #$80
	cmp lsign
	bne lcmpdone
	dey
	lda op2
	cmp (tmp),y
	bne lcmpdone
	dey
	lda op1+1
	cmp (tmp),y
	bne lcmpdone
	dey
	lda op1
	cmp (tmp),y
	rts

; ---- workspace --------------------------------------------------------
; Plain .dsb in the text section (NOT .bss - XA interleaves .bss symbol
; addresses with the surrounding code, which self-modifies the routines).
; 16 bytes of zeros in the image; must stay AFTER the code.

+lscratch	.dsb 4	; constant-operand staging (the macros' LPTB_C path)
lwork		.dsb 4	; local copy of operand B (mul/div shift space)
lrem		.dsb 4	; division remainder accumulator
lsign		.dsb 2	; sign bookkeeping for the signed wrappers
ldtmp		.dsb 2	; division inner-loop scratch (must NOT reuse lsign:
			; the signed wrappers hold their flags there across
			; the ldiv32u/lmod32u call)
