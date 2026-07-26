
#ifndef OSDK_CUSTOM_STACK

;
; The stack lives in .bss: 256 bytes of RAM right above the loaded image,
; reserved but NOT emitted (it used to be a .dsb 256 in .text, costing 256
; bytes of file size and load time in every program).
; XA 2.0 automatically chains .bss immediately after .text, so the stack just
; lands above the code - no end-of-text marker or manual origin needed.
; It should be protected !
;
; Warning: If you use the malloc functions, the heap is by default defined
; after the stack location (osdk_stack+_stacksize)!
;
 .bss

osdk_stack
	.dsb 256

; osdk_end keeps its historical meaning: first free byte after everything
; the program owns, stack included.
osdk_end

#else

 .text

osdk_end

#endif OSDK_CUSTOM_STACK
