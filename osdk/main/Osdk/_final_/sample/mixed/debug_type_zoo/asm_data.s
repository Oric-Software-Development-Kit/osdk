;
; asm_data.s - data DEFINED IN ASSEMBLER and exported to C.
;
; The leading underscore is the C linkage name: the assembler label '_g_palette'
; is the C symbol 'g_palette'. The linker resolves the references from world.c /
; main.c to the addresses these labels end up at.
;

; extern unsigned char g_palette[8];
_g_palette
	.byt 0,1,2,3,4,5,6,7

; extern unsigned int g_asm_ticks;   (16-bit, little endian)
_g_asm_ticks
	.word 0
