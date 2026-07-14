/* General integer arithmetic, promotions and comparisons. */
#include "testkit.h"

int gi, gj, gk;
unsigned int gu, gv;
char gc; unsigned char guc;

void main(void)
{
	tk_begin("arith");

	gi = 1000; gj = 33;
	tk_check_eq(gi + gj, 1033, "add");
	tk_check_eq(gi - gj, 967, "sub");
	tk_check_eq(gi * gj, 33000u, "mul-wrap");     /* 33000 wraps to -32536 as int */
	tk_check_eq(gi / gj, 30, "div");
	tk_check_eq(gi % gj, 10, "mod");

	gi = -1000; gj = 33;
	tk_check_eq(gi / gj, -30, "div-neg");
	tk_check_eq(gi % gj, (unsigned int)-10, "mod-neg");

	gu = 50000u; gv = 7;
	tk_check_eq(gu / gv, 7142u, "udiv");
	tk_check_eq(gu % gv, 6u, "umod");

	gi = 0x1234;
	tk_check_eq(gi << 1, 0x2468, "shl1");
	tk_check_eq(gi << 4, 0x2340, "shl4");
	tk_check_eq(gi >> 4, 0x0123, "shr4");
	gi = -16;
	tk_check_eq(gi >> 2, (unsigned int)-4, "sar");    /* arithmetic shift on signed */
	gu = 0xFFF0u;
	tk_check_eq(gu >> 2, 0x3FFCu, "lsr");             /* logical shift on unsigned */

	gu = 0xF0F0u; gv = 0x3C3Cu;
	tk_check_eq(gu & gv, 0x3030u, "and");
	tk_check_eq(gu | gv, 0xFCFCu, "or");
	tk_check_eq(gu ^ gv, 0xCCCCu, "xor");
	tk_check_eq(~gu, 0x0F0Fu, "com");

	gi = -5; gj = 3;
	tk_check(gi < gj, "lt-signed");
	tk_check(gj > gi, "gt-signed");
	gu = 0x8000u; gv = 3;
	tk_check(gu > gv, "gt-unsigned");                 /* would fail if compared signed */

	gc = (char)200;                                    /* chars are unsigned-ish 8bit */
	guc = 200;
	tk_check_eq(guc + 100, 300u, "uchar-promote");
	guc = 0xFF;
	guc++;
	tk_check_eq(guc, 0, "uchar-wrap");

	gi = 3;
	tk_check_eq(gi++ * 10, 30, "postinc");
	tk_check_eq(gi, 4, "postinc2");
	tk_check_eq(--gi * 10, 30, "predec");

	tk_end();
}
