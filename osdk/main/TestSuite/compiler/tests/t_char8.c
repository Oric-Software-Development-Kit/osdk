/* Native 8-bit (char) arithmetic - Phase 1: + - & | ^ on unsigned char.
   Verifies the byte-narrowing codegen (ADDB/SUBB/ANDB/ORB/XORB) against
   known results, including 8-bit wraparound. */
#include "testkit.h"

static unsigned char a, b, c;

void main(void)
{
	tk_begin("char8");

	/* var OP var */
	a = 200; b = 100; c = a + b;            /* 300 -> 44 (wrap) */
	tk_check_eq(c, 44, "add-wrap");
	a = 100; b = 200; c = a - b;            /* -100 -> 156 */
	tk_check_eq(c, 156, "sub-wrap");
	a = 0xF0; b = 0x3C; c = a & b;
	tk_check_eq(c, 0x30, "and");
	a = 0xF0; b = 0x0F; c = a | b;
	tk_check_eq(c, 0xFF, "or");
	a = 0xFF; b = 0x0F; c = a ^ b;
	tk_check_eq(c, 0xF0, "xor");

	/* var OP const */
	a = 250; c = a + 10;                    /* 260 -> 4 */
	tk_check_eq(c, 4, "addk-wrap");
	a = 5;   c = a - 9;                      /* -4 -> 252 */
	tk_check_eq(c, 252, "subk-wrap");
	a = 0xC3; c = a & 0x0F;
	tk_check_eq(c, 0x03, "andk");
	a = 0x0F; c = a | 0x80;
	tk_check_eq(c, 0x8F, "ork");
	a = 0xAA; c = a ^ 0xFF;
	tk_check_eq(c, 0x55, "xork");

	/* result feeding another char op (chained) */
	a = 10; b = 20; c = (a + b) & 0x0F;      /* 30 & 15 = 14 */
	tk_check_eq(c, 14, "chain");

	/* the low byte must be correct even when operands' high bytes differ:
	   here everything is char so this just re-confirms no stray high byte */
	a = 0; b = 1; c = a - b;                 /* -1 -> 255 */
	tk_check_eq(c, 255, "sub-borrow");

	/* Phase 2: unary ~ , unary - , << 1 */
	a = 0x0F; c = ~a;                        /* ~0x0F -> 0xF0 */
	tk_check_eq(c, 0xF0, "com");
	a = 1;    c = -a;                        /* -1 -> 255 */
	tk_check_eq(c, 255, "neg1");
	a = 5;    c = -a;                        /* -5 -> 251 */
	tk_check_eq(c, 251, "neg5");
	a = 0x40; c = a << 1;                    /* 0x80 */
	tk_check_eq(c, 0x80, "shl");
	a = 0x80; c = a << 1;                    /* 0x100 -> 0x00 (wrap) */
	tk_check_eq(c, 0x00, "shl-wrap");
	a = 0xFF; c = a << 1;                    /* 0x1FE -> 0xFE */
	tk_check_eq(c, 0xFE, "shl-ff");

	/* mixed with Phase 1 ops (these exercise the word path for the inner op
	   but must still produce the right char result) */
	a = 0x03; c = (~a) & 0x0F;               /* 0xFC & 0x0F -> 0x0C */
	tk_check_eq(c, 0x0C, "com-and");
	a = 0x09; c = (a << 1) + 1;              /* 0x12 + 1 -> 0x13 */
	tk_check_eq(c, 0x13, "shl-add");

	tk_end();
}
