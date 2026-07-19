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

	/* (x & mask) tests: the AND result is byte-bounded, so the compare
	   narrows to a single-byte test (and the operand widen is elided) */
	a = 0x85;
	c = (a & 0x80) ? 1 : 2;
	tk_check_eq(c, 1, "andmask-set");
	c = (a & 0x02) ? 1 : 2;
	tk_check_eq(c, 2, "andmask-clear");
	b = 0x7F;
	c = (b & 0x80) ? 1 : 2;
	tk_check_eq(c, 2, "andmask-top");
	b = 0x05;
	c = ((a & b) != 0) ? 1 : 2;              /* var & var */
	tk_check_eq(c, 1, "andvar-ne");
	c = ((a & 0x42) == 0) ? 1 : 2;           /* 0x85 & 0x42 = 0 */
	tk_check_eq(c, 1, "andmask-eq0");
	c = ((a & 0x07) == 5) ? 1 : 2;           /* nonzero byte const compare */
	tk_check_eq(c, 1, "andmask-eqn");

	/* the xtime() idiom: copy, shift, test bit 7 of the COPY - the carry
	   fold turns the whole (copy & 0x80) test into the asl's carry */
	a = 0x91; b = a; a = a << 1;
	c = (b & 0x80) ? 1 : 2;
	tk_check_eq(a, 0x22, "xtime-shift");     /* 0x91<<1 wraps to 0x22 */
	tk_check_eq(c, 1, "xtime-carryset");
	a = 0x41; b = a; a = a << 1;
	c = (b & 0x80) ? 1 : 2;
	tk_check_eq(a, 0x82, "xtime-shift2");
	tk_check_eq(c, 2, "xtime-carryclear");

	/* while(v--): the post-decrement collapse (ldx/dex/stx/inx) must test
	   the ORIGINAL value and wrap the stored one - boundary cases c==1
	   (one iteration, not zero) and c==0 (no iteration, wraps to 255) */
	a = 3; b = 0; while (a--) b++;
	tk_check_eq(b, 3, "postdec-count");
	tk_check_eq(a, 255, "postdec-wrap");     /* last test decrements 0 */
	a = 1; b = 0; while (a--) b++;
	tk_check_eq(b, 1, "postdec-one");
	a = 0; b = 77; while (a--) b = 0;
	tk_check_eq(b, 77, "postdec-zero");
	tk_check_eq(a, 255, "postdec-zerowrap");

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

	/* Phase 4: unsigned char >> 1 must be a LOGICAL shift (bit 7 <- 0);
	   an arithmetic byte shift would turn 0x80 into 0xC0. */
	a = 0x80; c = a >> 1;                    /* 0x40, not 0xC0 */
	tk_check_eq(c, 0x40, "shr-hi");
	a = 0xFF; c = a >> 1;                    /* 0x7F */
	tk_check_eq(c, 0x7F, "shr-ff");
	a = 0x01; c = a >> 1;                    /* 0x00 */
	tk_check_eq(c, 0x00, "shr-1");
	a = 0x55; c = a >> 1;                    /* 0x2A */
	tk_check_eq(c, 0x2A, "shr-55");

	/* Phase 3: char comparisons ( == != < > <= >= ), var-var and var-const.
	   The >=128 cases prove the byte compare is UNSIGNED (a signed byte
	   compare would treat 0x80..0xFF as negative and invert the result). */
	{
		unsigned char x, y;
		int t;
		x = 10; y = 20;
		t = 0; if (x == y) t = 1; tk_check_eq(t, 0, "eq-f");
		t = 0; if (x != y) t = 1; tk_check_eq(t, 1, "ne-t");
		t = 0; if (x <  y) t = 1; tk_check_eq(t, 1, "lt-t");
		t = 0; if (x >  y) t = 1; tk_check_eq(t, 0, "gt-f");
		t = 0; if (x <= y) t = 1; tk_check_eq(t, 1, "le-t");
		t = 0; if (x >= y) t = 1; tk_check_eq(t, 0, "ge-f");
		x = 200; y = 200;
		t = 0; if (x == y) t = 1; tk_check_eq(t, 1, "eq-t");
		t = 0; if (x <= y) t = 1; tk_check_eq(t, 1, "le-eq");
		t = 0; if (x <  y) t = 1; tk_check_eq(t, 0, "lt-eq");
		x = 200;
		t = 0; if (x > 100) t = 1; tk_check_eq(t, 1, "gt-k-hi");  /* 200>100 */
		t = 0; if (x < 100) t = 1; tk_check_eq(t, 0, "lt-k-hi");
		x = 0xFF; y = 0x01;
		t = 0; if (x > y) t = 1; tk_check_eq(t, 1, "gt-ff");      /* 255>1 */
		t = 0; if (x >= 0x80) t = 1; tk_check_eq(t, 1, "ge-k-80");
		x = 65;
		t = 0; if (x == 65) t = 1; tk_check_eq(t, 1, "eqk");
		t = 0; if (x != 65) t = 1; tk_check_eq(t, 0, "nek");
	}

	tk_end();
}
