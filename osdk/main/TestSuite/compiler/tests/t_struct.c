/* Struct copies in every addressing shape: exercises the ASGNS macro family,
 * including the eight -O3-only variants (CD DD AD YD CY DY YY YZ) added in
 * 2026-07 after they were found missing. */
#include "testkit.h"

struct S { int a; char b; int c; };

struct S gs, gt;
struct S gsarr[4];
struct S *gsp, *gsp2;
int gidx;

/* consecutive initialized const structs, for the data-packing check */
const struct S ka = { 1, 2, 3 };
const struct S kb = { 4, 5, 6 };

void fill(struct S *p, int base)
{
	p->a = base;
	p->b = (char)(base & 0xFF);
	p->c = base + 1;
}

int same(struct S *p, int base)
{
	return p->a == base
	    && p->b == (char)(base & 0xFF)
	    && p->c == base + 1;
}

void via_param_dst(struct S *p)      { *p = gs;    }   /* ASGNS_CY at -O3 */
void via_param_src(struct S *p)      { gt = *p;    }
void via_params   (struct S *p, struct S *q) { *p = *q; } /* ASGNS_YY at -O3 */
void via_param_idx(struct S *p, int i) { gsarr[i] = *p; } /* ASGNS_YZ at -O3 */
void param_to_gptr(struct S *p)      { *gsp = *p;  }   /* ASGNS_YD at -O3 */
void gptr_to_param(struct S *p)      { *p = *gsp;  }   /* ASGNS_DY at -O3 */
void via_local    (void)             { struct S l; fill(&l, 0x4d0); *gsp = l; } /* ASGNS_AD */
struct S ret_struct(void)            { return gs; }
void take_struct(struct S v)         { gt = v; }

void main(void)
{
	tk_begin("struct");

	fill(&gsarr[2], 0x100);
	gs = gsarr[2];                       /* global = global element */
	tk_check(same(&gs, 0x100), "cc");

	fill(&gs, 0x210);
	gsp = &gsarr[0];
	*gsp = gs;                           /* through global pointer */
	tk_check(same(&gsarr[0], 0x210), "cd");

	fill(&gsarr[3], 0x320);
	gsp = &gt;
	gsp2 = &gsarr[3];
	*gsp = *gsp2;                        /* ptr = ptr : ASGNS_DD at -O3 */
	tk_check(same(&gt, 0x320), "dd");

	gsp = &gsarr[1];
	via_local();                         /* *gsp = local struct */
	tk_check(same(&gsarr[1], 0x4d0), "ad");

	fill(&gs, 0x5e0);
	via_param_dst(&gsarr[2]);            /* *param = global */
	tk_check(same(&gsarr[2], 0x5e0), "cy");

	fill(&gsarr[0], 0x6f0);
	via_param_src(&gsarr[0]);            /* global = *param */
	tk_check(same(&gt, 0x6f0), "yc");

	fill(&gt, 0x6f1);
	gsp = &gsarr[0];
	param_to_gptr(&gt);                  /* *globalptr = *param */
	tk_check(same(&gsarr[0], 0x6f1), "yd");

	fill(&gsarr[1], 0x6f2);
	gsp = &gsarr[1];
	gptr_to_param(&gt);                  /* *param = *globalptr */
	tk_check(same(&gt, 0x6f2), "dy");

	fill(&gsarr[3], 0x701);
	via_params(&gs, &gsarr[3]);          /* *param = *param */
	tk_check(same(&gs, 0x701), "yy");

	fill(&gt, 0x812);
	via_param_idx(&gt, 2);               /* computed dest = *param */
	tk_check(same(&gsarr[2], 0x812), "yz");

	fill(&gs, 0x923);
	gt = ret_struct();                   /* struct return */
	tk_check(same(&gt, 0x923), "ret");

	fill(&gs, 0xa34);
	take_struct(gs);                     /* struct parameter by value */
	tk_check(same(&gt, 0xa34), "param");

	/* consecutive const struct initializers must emit exactly sizeof
	   bytes each - no padding or leaked values between the labels */
	tk_check_eq((unsigned int)((char*)&kb - (char*)&ka), sizeof(struct S), "data-packing");

	tk_end();
}
