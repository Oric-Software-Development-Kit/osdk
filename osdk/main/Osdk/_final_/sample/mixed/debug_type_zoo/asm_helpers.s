;
; asm_helpers.s - routines called FROM C.
;
; Calling convention (see the OSDK 'mixed' hello-world samples):
;   - the C function Foo() is the assembler label _Foo
;   - every parameter occupies a 2-byte slot on the software stack 'sp':
;     the first parameter is at (sp)+0, the second at (sp)+2, and so on
;     (a char sits in the low byte of its slot).
;
; These routines use self-modifying code for pointer access, so they need no
; zero-page scratch of their own.
;


; void AsmXorChecksum(unsigned char *data, int count);
;   sp+0 : data pointer (2 bytes)
;   sp+2 : count        (2 bytes; only the low byte is used, so count < 256)
; Result: g_asm_checksum = data[0] ^ data[1] ^ ... ^ data[count-1]
_AsmXorChecksum
	; Patch the data pointer into the absolute operand of 'read_byte'.
	ldy #0
	lda (sp),y
	sta read_byte+1
	iny
	lda (sp),y
	sta read_byte+2

	; X = count (low byte) = down-counter, A = running XOR, Y = index.
	ldy #2
	lda (sp),y
	tax
	lda #0
	ldy #0
xor_loop
	cpx #0
	beq xor_done
read_byte
	eor $1234,y          ; $1234 is patched above to point at 'data'
	iny
	dex
	jmp xor_loop
xor_done
	sta _g_asm_checksum   ; write the C-defined global
	rts


; void AsmTick(void);    g_asm_ticks++  (16-bit)
_AsmTick
	inc _g_asm_ticks
	bne tick_done
	inc _g_asm_ticks+1
tick_done
	rts
