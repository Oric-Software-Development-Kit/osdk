; --------------------------------------
;           .bin and .dsb demo
; --------------------------------------
;
; This sample shows you how to use ".bin" to directly include a binary file into your executable.
;

	.text

_main
	; Switch to HIRES
	jsr _hires

	; Copy the image data to screen
	lda #<ImageData
	sta tmp0+0
	lda #>ImageData
	sta tmp0+1

	lda #<$a000
	sta tmp1+0
	lda #>$a000
	sta tmp1+1

	ldx #18
loop_outer
	ldy #0
loop_inner
	lda (tmp0),y
	sta (tmp1),y
	iny
	bne loop_inner
	inc tmp0+1
	inc tmp1+1
	dex
	bne loop_outer
	rts

;
; Syntax:  .bin offset, length, "filename"
;   offset  = byte position to start reading from
;   length  = number of bytes (0 = rest of file)
ImageData
	.bin 40*35, 40*115, "BUILD/picture.bin"    ; 115 lines of the picture, starting at line 35
	.dsb 40,16+1                               ; One line worth of red paper attribute to pad
	
