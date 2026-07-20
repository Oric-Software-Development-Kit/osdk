/* 32-bit long support - known-answer checks for the L codegen and the
   long32.s runtime (add/sub/logic/mul/div/mod/shift/compare/convert).
   Every value is checked in two 16-bit halves so a failure pinpoints
   which half of which operation broke. */
#include "testkit.h"

static long ga, gb, gr;
static unsigned long ua, ur;
static int i16;
static unsigned int u16;
static long *gp;

/* initialized long globals must occupy the full 4 bytes (regression: a
   static long initializer used to emit a single truncated 2-byte word). */
static long init_s   = 0x12345678;
static long init_neg = -100000L;
static unsigned long init_u = 0x89ABCDEFuL;
static long init_arr[3] = { 100000L, 0x7FFFFFFFL, -1L };
static long rng_seed = 0x55aa55aa;             /* the shuffle-benchmark LCG */
static int rng_next(void) { rng_seed = 69069 * rng_seed + 1234567; return 0x7fff & rng_seed; }

#define CHECK32(v, hi, lo, name) \
	do { tk_check_eq((unsigned)((v) >> 16), (hi), name "-hi"); \
	     tk_check_eq((unsigned)(v), (lo), name "-lo"); } while (0)

static long lsum(long x, long y)
{
	return x + y;
}

static void store_via_ptr(long *p, long v)
{
	*p = v;
}

static long load_via_ptr(long *p)
{
	return *p;
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

	/* inline long compare against a constant: gen.c LTLK/GELK/... - canonicalise
	   const-on-left (flip relation) and fold a<=k -> a<k+1, a>k -> a>=k+1.
	   Cover every relation, both operand orders, signed & unsigned, negative and
	   multi-byte constants, the unsigned high-bit case, and the +1-overflow guard
	   (a<=MAX must stay always-true via the routine fallback). */
	{
		long s; unsigned long u; int c;
		s=99;    c=0; if (s <  100) c=1; tk_check_eq(c,1,"clt-below");
		s=100;   c=0; if (s <  100) c=1; tk_check_eq(c,0,"clt-eq");
		s=-5;    c=0; if (s <  100) c=1; tk_check_eq(c,1,"clt-neg");
		s=100;   c=0; if (s <= 100) c=1; tk_check_eq(c,1,"cle-eq");     /* fold <101 */
		s=101;   c=0; if (s <= 100) c=1; tk_check_eq(c,0,"cle-above");
		s=101;   c=0; if (s >  100) c=1; tk_check_eq(c,1,"cgt-above");  /* fold >=101 */
		s=100;   c=0; if (s >  100) c=1; tk_check_eq(c,0,"cgt-eq");
		s=100;   c=0; if (s >= 100) c=1; tk_check_eq(c,1,"cge-eq");
		s=99;    c=0; if (s >= 100) c=1; tk_check_eq(c,0,"cge-below");
		s=100;   c=0; if (s == 100) c=1; tk_check_eq(c,1,"ceq-y");
		s=99;    c=0; if (s == 100) c=1; tk_check_eq(c,0,"ceq-n");
		s=99;    c=0; if (s != 100) c=1; tk_check_eq(c,1,"cne-y");
		s=100;   c=0; if (s != 100) c=1; tk_check_eq(c,0,"cne-n");
		/* constant on the LEFT (canonicalisation + relation flip) */
		s=101;   c=0; if (100 <  s) c=1; tk_check_eq(c,1,"clft-lt-y");
		s=100;   c=0; if (100 <  s) c=1; tk_check_eq(c,0,"clft-lt-n");
		s=100;   c=0; if (100 >= s) c=1; tk_check_eq(c,1,"clft-ge-y");
		s=101;   c=0; if (100 >= s) c=1; tk_check_eq(c,0,"clft-ge-n");
		/* negative constant (signed MSB is 0xFF, flipped to 0x7F) */
		s=-100;  c=0; if (s < -50) c=1; tk_check_eq(c,1,"cneg-y");
		s=-40;   c=0; if (s < -50) c=1; tk_check_eq(c,0,"cneg-n");
		/* multi-byte constants crossing byte boundaries */
		s=99999; c=0; if (s < 100000) c=1; tk_check_eq(c,1,"cbig-y");
		s=100001;c=0; if (s < 100000) c=1; tk_check_eq(c,0,"cbig-n");
		s=-100001;c=0;if (s < -100000) c=1; tk_check_eq(c,1,"cbigneg-y");
		s=-99999;c=0; if (s < -100000) c=1; tk_check_eq(c,0,"cbigneg-n");
		/* unsigned: high-bit values must NOT be read as negative */
		u=0x7FFFFFFFul; c=0; if (u <  0x80000000ul) c=1; tk_check_eq(c,1,"ult-y");
		u=0x80000001ul; c=0; if (u <  0x80000000ul) c=1; tk_check_eq(c,0,"ult-n");
		u=0x80000000ul; c=0; if (u >= 0x80000000ul) c=1; tk_check_eq(c,1,"uge-y");
		u=0x7FFFFFFFul; c=0; if (u >= 0x80000000ul) c=1; tk_check_eq(c,0,"uge-n");
		u=100;   c=0; if (u <= 100ul) c=1; tk_check_eq(c,1,"ule-y");
		u=101;   c=0; if (u <= 100ul) c=1; tk_check_eq(c,0,"ule-n");
		/* +1-overflow guard: a<=TYPE_MAX is always true (must fall back, not wrap) */
		u=0xFFFFFFFFul; c=0; if (u <= 0xFFFFFFFFul) c=1; tk_check_eq(c,1,"ule-max");
		u=12345ul;      c=0; if (u <= 0xFFFFFFFFul) c=1; tk_check_eq(c,1,"ule-max2");
		s=0x7FFFFFFF;   c=0; if (s <= 0x7FFFFFFF)    c=1; tk_check_eq(c,1,"sle-max");
		s=-1;           c=0; if (s <= 0x7FFFFFFF)    c=1; tk_check_eq(c,1,"sle-max2");
	}

	/* fused `dst = mem_long +/- const` (gen.c ADDLKM/SUBLKM): reads the source
	   long and writes a different destination with an inline carry/borrow chain,
	   no jsr ladd32/lsub32. Static source (_C) and frame-local source (_A). */
	ga = 100000;    gr = ga + 6;     CHECK32(gr, 0x0001, 0x86A6u, "fk-add-c");
	ga = 0x0000FFFF;gr = ga + 1;     CHECK32(gr, 0x0001, 0x0000u, "fk-add-carry");
	ga = 100006;    gr = ga - 6;     CHECK32(gr, 0x0001, 0x86A0u, "fk-sub-c");
	ga = 0x00010000;gr = ga - 1;     CHECK32(gr, 0x0000, 0xFFFFu, "fk-sub-borrow");
	ga = -100;      gr = ga + 6;     CHECK32(gr, 0xFFFFu, 0xFFA2u, "fk-add-neg");
	{
		long fa, fb;
		fa = 200000;    fb = fa + 70000; CHECK32(fb, 0x0004, 0x1EB0u, "fk-add-a");
		fa = 0x00FFFFFF;fb = fa + 1;     CHECK32(fb, 0x0100, 0x0000u, "fk-add-a-carry");
		fa = 270000;    fb = fa - 70000; CHECK32(fb, 0x0003, 0x0D40u, "fk-sub-a");
		fa = 5;         fb = fa - 10;    CHECK32(fb, 0xFFFFu, 0xFFFBu, "fk-sub-a-borrow");
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

	/* dereference a long through pointers held at every storage class:
	   global ptr (D mode), param ptr (Y mode) and local ptr (Z mode).
	   D/Y used to store/load the pointer's own slot instead of through it. */
	{
		long *lp;
		gp = &ga;
		*gp = 0x11223344;                       /* store via GLOBAL ptr (ASGNL D) */
		CHECK32(ga, 0x1122u, 0x3344u, "derefStoreD");
		gr = *gp;                               /* load  via GLOBAL ptr (INDIRL D) */
		CHECK32(gr, 0x1122u, 0x3344u, "derefLoadD");
		gb = 0;
		store_via_ptr(&gb, (long)0x8899AABBu);  /* store via PARAM ptr (ASGNL Y) */
		CHECK32(gb, 0x8899u, 0xAABBu, "derefStoreY");
		gr = load_via_ptr(&gb);                 /* load  via PARAM ptr (INDIRL Y) */
		CHECK32(gr, 0x8899u, 0xAABBu, "derefLoadY");
		gr = 0;
		lp = &gr;
		*lp = 0x0F0E0D0C;                       /* store via LOCAL ptr (ASGNL Z) */
		CHECK32(gr, 0x0F0Eu, 0x0D0Cu, "derefStoreZ");
	}

	/* initialized long globals: full 4-byte value, not a truncated word */
	CHECK32(init_s,      0x1234u, 0x5678u, "initS");
	CHECK32(init_neg,    0xFFFEu, 0x7960u, "initNeg");
	CHECK32(init_u,      0x89ABu, 0xCDEFu, "initU");
	CHECK32(init_arr[0], 0x0001u, 0x86A0u, "initArr0");
	CHECK32(init_arr[1], 0x7FFFu, 0xFFFFu, "initArr1");
	CHECK32(init_arr[2], 0xFFFFu, 0xFFFFu, "initArr2");
	/* LCG that mutates an initialized static long through a function */
	tk_check_eq(rng_next(), 4521, "rng1");
	CHECK32(rng_seed, 0x957Du, 0x11A9u, "rngSeed1");
	tk_check_eq(rng_next(), 4060, "rng2");
	CHECK32(rng_seed, 0x20C8u, 0x8FDCu, "rngSeed2");

	tk_end();
}
