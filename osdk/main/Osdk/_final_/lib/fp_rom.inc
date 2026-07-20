;
; Oric BASIC ROM floating-point entry points.
;
#ifndef FP_ROM_INC
#define FP_ROM_INC

#define load_acc1	$DE7B
#define load_acc2	$DD51
#define store_acc	$DEAD
#define fadd		$DB25
#define fsub		$DB0E
#define fmul		$DCF0
#define fdiv		$DDE7
#define fneg		$E271
#define fcomp		$DF4C
#define givayf		$DF40	; UNSIGNED 16bit A(high)/Y(low) to FPA (forces the sign
							; byte positive). The historical cif value $DF24 was
							; wrong: that is SGN's tail, converting only the signed
							; 8bit value in A.

#endif
