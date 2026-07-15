/* Bitfield store/read in every signedness shape: guards the peephole
   optimizer's immediate normalization (a strtol overflow once turned the
   field-clear mask #>($fffff0ff) into #255, so stores merged the old field
   bits instead of clearing them - found via cc65 bug1267). */
#include "testkit.h"

typedef int i16;
typedef unsigned int u16;
typedef signed int s16;

static struct B { i16 i : 4; u16 u : 4; s16 s : 4; } b = {1, 2, 3};
static struct W { unsigned int lo : 5; unsigned int mid : 6; unsigned int hi : 5; } w;

void main(void)
{
	tk_begin("bitfield");

	tk_check(b.i == 1, "init-i");
	tk_check(b.u == 2, "init-u");
	tk_check(b.s == 3, "init-s");

	b.i = -1;
	b.u = -2;
	b.s = -3;
	tk_check(b.i == -1, "store-i");
	tk_check(b.u == 14, "store-u");
	tk_check(b.s == -3, "store-s");

	w.lo = 21;
	w.mid = 45;
	w.hi = 17;
	tk_check(w.lo == 21, "w-lo");
	tk_check(w.mid == 45, "w-mid");
	tk_check(w.hi == 17, "w-hi");

	w.mid = 0;
	tk_check(w.lo == 21 && w.hi == 17, "w-clear-neighbors");

	tk_end();
}
