/* 32-bit long support - known-answer checks for the L codegen and the
   long32.s runtime (add/sub/logic/mul/div/mod/shift/compare/convert).
   Every value is checked in two 16-bit halves so a failure pinpoints
   which half of which operation broke. */
#include "testkit.h"

static long ga, gb, gr;
static unsigned long ua, ur;
static int i16;
static unsigned int u16;

#define CHECK32(v, hi, lo, name) \
	do { tk_check_eq((unsigned)((v) >> 16), (hi), name "-hi"); \
	     tk_check_eq((unsigned)(v), (lo), name "-lo"); } while (0)

static long lsum(long x, long y)
{
	return x + y;
}

void main(void)
{
	tk_begin("long");

	/* literals & moves */
	ga = 100000;                    /* 0x000186A0 */
	CHECK32(ga, 0x0001, 0x86A0u, "lit");
	ga = -100000;                   /* 0xFFFE7960 */
	CHECK32(ga, 0xFFFEu, 0x7960u, "litneg");
	ua = 0x89ABCDEFu;
	CHECK32(ua, 0x89ABu, 0xCDEFu, "litu");

	/* add / sub with carries across the word boundary */
	ga = 100000; gb = 70000; gr = ga + gb;          /* 170000 = 0x29810 */
	CHECK32(gr, 0x0002, 0x9810u, "add");
	gr = ga - gb;                                   /* 30000 */
	CHECK32(gr, 0x0000, 30000u, "sub");
	gr = gb - ga;                                   /* -30000 = 0xFFFF8AD0 */
	CHECK32(gr, 0xFFFFu, 0x8AD0u, "subneg");
	ga = 0x0000FFFF; gr = ga + 1;                   /* carry into high word */
	CHECK32(gr, 0x0001, 0x0000u, "carry");
	gr = gr - 1;
	CHECK32(gr, 0x0000, 0xFFFFu, "borrow");

	/* compound assignment with a constant exercises gen.c's in-place RMW
	   (ADDLK/SUBLK: adc/sbc chain straight on the destination). Check
	   carry/borrow through every byte boundary, on a static long (ADDLK_C)
	   and a frame-local long (ADDLK_A). */
	{
		long la;                                /* frame-local -> ADDLK_A */
		ga = 100000; ga += 6;                   CHECK32(ga, 0x0001, 0x86A6u, "pe-c");
		ga = 0x0000FFFF; ga += 1;               CHECK32(ga, 0x0001, 0x0000u, "pe-carry16");
		ga = 0x00FFFFFF; ga += 1;               CHECK32(ga, 0x0100, 0x0000u, "pe-carry24");
		ga = 100006; ga -= 6;                   CHECK32(ga, 0x0001, 0x86A0u, "me-c");
		ga = 0x00010000; ga -= 1;               CHECK32(ga, 0x0000, 0xFFFFu, "me-borrow16");
		ga = 0x01000000; ga -= 1;               CHECK32(ga, 0x00FF, 0xFFFFu, "me-borrow24");
		ga = 5; ga += 0;                        CHECK32(ga, 0x0000, 5u,      "pe-zero");
		ga = -100; ga += 6;                     CHECK32(ga, 0xFFFFu, 0xFFA2u,"pe-neg"); /* -94 */
		la = 200000; la += 70000;               CHECK32(la, 0x0004, 0x1EB0u, "pe-a");
		la = 0x00FFFFFF; la += 1;               CHECK32(la, 0x0100, 0x0000u, "pe-a-carry24");
		la = 270000; la -= 70000;               CHECK32(la, 0x0003, 0x0D40u, "me-a");
		la = 0; la -= 1;                        CHECK32(la, 0xFFFFu, 0xFFFFu,"me-a-borrow");
	}

	/* logic + unary */
	ga = 0x0F0F5AA5; gb = 0x00FF00FF;
	gr = ga & gb; CHECK32(gr, 0x000F, 0x00A5u, "and");
	gr = ga | gb; CHECK32(gr, 0x0FFF, 0x5AFFu, "or");
	gr = ga ^ gb; CHECK32(gr, 0x0FF0, 0x5A5Au, "xor");
	gr = ~ga;     CHECK32(gr, 0xF0F0u, 0xA55Au, "com");
	gr = -ga;     CHECK32(gr, 0xF0F0u, 0xA55Bu, "neg");

	/* multiply */
	ga = 10000; gb = 2000; gr = ga * gb;            /* 20,000,000 = 0x1312D00 */
	CHECK32(gr, 0x0131, 0x2D00u, "mul");
	ga = -3; gb = 100000; gr = ga * gb;             /* -300000 = 0xFFFB6C20 */
	CHECK32(gr, 0xFFFBu, 0x6C20u, "mulneg");

	/* divide / modulo, unsigned */
	ua = 20000000u; ur = ua / 1119u;                /* 17873 r 113 */
	CHECK32(ur, 0x0000, 17873u, "udiv");
	ur = ua % 1119u;
	CHECK32(ur, 0x0000, 113u, "umod");
	ua = 0x89ABCDEFu; ur = ua / 0x10000u;           /* high word */
	CHECK32(ur, 0x0000, 0x89ABu, "udiv64k");

	/* divide / modulo, signed (C89: quotient toward zero, rem = dividend sign) */
	ga = 169995; gr = ga / 1000;                    /* 169 */
	CHECK32(gr, 0x0000, 169u, "div");
	gr = ga % 1000;                                 /* 995 */
	CHECK32(gr, 0x0000, 995u, "mod");
	ga = -169995; gr = ga / 1000;                   /* -169 */
	CHECK32(gr, 0xFFFFu, (unsigned)-169, "divneg");
	gr = ga % 1000;                                 /* -995 */
	CHECK32(gr, 0xFFFFu, (unsigned)-995, "modneg");
	ga = 169995; gr = ga / -1000;                   /* -169 */
	CHECK32(gr, 0xFFFFu, (unsigned)-169, "divnegd");

	/* shifts */
	ga = 1; gr = ga << 20;                          /* 0x00100000 */
	CHECK32(gr, 0x0010, 0x0000u, "shl");
	ua = 0x89ABCDEFu; ur = ua >> 12;                /* 0x00089ABC */
	CHECK32(ur, 0x0008, 0x9ABCu, "shrl");
	ga = -4096; gr = ga >> 4;                       /* arithmetic: -256 */
	CHECK32(gr, 0xFFFFu, 0xFF00u, "asr");

	/* compares - each both ways */
	ga = 100000; gb = 70000;
	tk_check_eq(ga > gb, 1, "gt1");
	tk_check_eq(gb > ga, 0, "gt0");
	tk_check_eq(ga == 100000, 1, "eq1");
	tk_check_eq(ga == gb, 0, "eq0");
	tk_check_eq(ga != gb, 1, "ne1");
	ga = -100000;
	tk_check_eq(ga < 0, 1, "ltneg");
	tk_check_eq(ga < gb, 1, "lt1");
	tk_check_eq(gb < ga, 0, "lt0");
	ua = 0x80000000u;
	tk_check_eq(ua > 1u, 1, "ugt");     /* unsigned: MSB set is BIG */
	tk_check_eq(ua < 0xFFFFFFFFu, 1, "ult");

	/* conversions */
	i16 = -5; gr = i16;                             /* sign extend */
	CHECK32(gr, 0xFFFFu, 0xFFFBu, "cswl");
	u16 = 0x8001u; gr = u16;                        /* zero extend */
	CHECK32(gr, 0x0000, 0x8001u, "czwl");
	ga = 0x12345678; i16 = (int)ga;                 /* truncate */
	tk_check_eq((unsigned)i16, 0x5678u, "clw");

	/* arrays, pointers, function args/returns (pi shapes) */
	{
		static long arr[5];
		long *lp;
		int j;
		for (j = 0; j < 5; j++)
			arr[j] = 100000 + j;                    /* computed store */
		CHECK32(arr[0], 0x0001, 0x86A0u, "arr0");
		CHECK32(arr[4], 0x0001, 0x86A4u, "arr4");
		j = 3;
		gr = arr[j] * 2;                            /* computed load */
		CHECK32(gr, 0x0003, 0x0D46u, "arrj");
		lp = &arr[2];
		*lp = *lp + 7;
		CHECK32(arr[2], 0x0001, 0x86A9u, "ptr");
		gr = lsum(arr[1], arr[2]);                  /* long args + return */
		CHECK32(gr, 0x0003, 0x0D4Au, "fnargs");
		gr = 0;
		for (j = 5; j > 0; --j)                     /* pi inner-loop shape */
			gr = gr * j + 10000 * (long)2;
		CHECK32(gr, 0x000A, 0x6040u, "spigot");   /* 680000 */
	}

	/* long arithmetic feeding int contexts and vice versa (pi shapes) */
	ga = 0; i16 = 14;
	gr = ga * i16 + 10000 * (long)2000;             /* 20,000,000 */
	CHECK32(gr, 0x0131, 0x2D00u, "mixmuladd");
	gr = gr % (i16 * 2 - 1);                        /* 20000000 % 27 = 20 */
	CHECK32(gr, 0x0000, 20u, "mixmod");

	tk_end();
}
