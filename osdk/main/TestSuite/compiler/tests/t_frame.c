/* Frame-resident operands at -O3: exercises SUBW_YYY, ANDW_YYY, ORW_YYY,
 * XORW_YYY, COMW_YY and INDIRW_DY, whose MACROS.H bodies generated invalid
 * assembly until the 2026-07 fix. Taking the address of a parameter keeps
 * it on the stack frame (blocks zero page register promotion), which is
 * what makes the compiler emit the _YY/_YYY variants. */
#include "testkit.h"

int gi;
int *gip;

int t_sub(int a, int b) { int *k = &a; a = a - b; return *k; }
int t_and(int a, int b) { int *k = &a; a = a & b; return *k; }
int t_or (int a, int b) { int *k = &a; a = a | b; return *k; }
int t_xor(int a, int b) { int *k = &a; a = a ^ b; return *k; }
int t_com(int a)        { int *k = &a; a = ~a;    return *k; }
int t_ind(int a)        { int *k = &a; a = *gip;  return *k; }

void main(void)
{
	tk_begin("frame");

	tk_check_eq(t_sub(100, 42), 58, "subw_yyy");
	tk_check_eq(t_and(0xF0F0, 0x3C3C), 0x3030u, "andw_yyy");
	tk_check_eq(t_or (0xF0F0, 0x3C3C), 0xFCFCu, "orw_yyy");
	tk_check_eq(t_xor(0xF0F0, 0x3C3C), 0xCCCCu, "xorw_yyy");
	tk_check_eq(t_com(0x1234), 0xEDCBu, "comw_yy");

	gi = 0x5678;
	gip = &gi;
	tk_check_eq(t_ind(0), 0x5678u, "indirw_dy");

	tk_end();
}
