


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
	; Clear the BSS section.
	;
	; C guarantees that statics start at zero. They used to get that for free:
	; uninitialised statics were reserved with .dsb inside .text, and .dsb in an
	; emitted segment writes zero bytes, so the zeroes arrived with the file - at
	; the cost of carrying (and loading) them from tape or disk. They now live in
	; .bss, which reserves without emitting, so the runtime has to do the zeroing.
	;
	; Only the AUTO-CHAINED run is cleared: __bss_clear_start/__bss_clear_end cover
	; the reservations made before any "* = $XXXX" pinned the .bss PC. A pinned
	; block is a deliberate placement (screen, overlay, hardware) and is none of
	; the CRT's business - which is also why __bss_end must NOT be used here, since
	; it is start+total-length and so spans those pinned blocks.
	;
	; Define OSDK_NO_BSS_CLEAR if your project manages .bss itself.
	;
	; The range cleared is __bss_clear_start up to osdk_stack: everything the
	; compiler reserved, but NOT the stack, which is the last thing in the natural
	; run and never needs zeroing. XA page-aligns __bss_clear_start, so this walks
	; whole pages with "iny" and needs no pointer arithmetic - the only self
	; modification is the high byte of the store, one patch per page. Rounding the
	; count up to a whole page can only ever spill into the stack, which is safe.
#ifdef OSDK_HAS_BSS		; hoisted by link65 only when a module reserves .bss
#ifndef OSDK_NO_BSS_CLEAR
#ifndef OSDK_CUSTOM_STACK
	ldx #>(osdk_stack - __bss_clear_start + 255)	; pages to clear (0 = nothing)
	beq osdk_bss_done
	lda #0
osdk_bss_page
	ldy #0
osdk_bss_store
	sta __bss_clear_start,y
	iny
	bne osdk_bss_store
	inc osdk_bss_store+2
	dex
	bne osdk_bss_page
osdk_bss_done
#endif
#endif
#endif




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

