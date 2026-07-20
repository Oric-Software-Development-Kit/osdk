


	.zero

#ifndef OSDK_ZP_START
#define OSDK_ZP_START $50
#endif
	*= OSDK_ZP_START

zp_compiler_save_start
#include "zp_crt.inc"
zp_compiler_save_end


	.text

osdk_start
    ;jmp osdk_start		; Comment out to not autostart the system

	;#include "adress.tmp"
	;*=$800

	;
	; Needs to clear the BSS section
	;




	; Debugger module id: stamp _osdk_dbg_module as the very first thing a module
	; runs (C or asm), so the VS Code debugger auto-switches to the matching overlay.
	; Transparent: only compiled when the build defines OSDK_MODULE_ID (per-overlay
	; -DOSDK_MODULE_ID=<id>); other OSDK programs never define it, so it's a no-op —
	; and the OSDK-namespaced name avoids clashing with a project's own 'MODULE'.
#ifdef OSDK_MODULE_ID
	lda #OSDK_MODULE_ID
	sta _osdk_dbg_module
#endif

	tsx
	lda #<osdk_stack
	sta sp
	lda #>osdk_stack
	sta sp+1
	ldy #0
	stx exitldx+1	; patch _exit's immediate: the saved hardware stack
			; pointer lives IN the ldx operand (no .byt needed)
	jmp _main

_exit
exitldx
	ldx #00
	txs
	rts

#include "fp_rom.inc"

