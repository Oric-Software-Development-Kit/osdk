/* 0b binary literals (added to the compiler in 2026-07; the benchmark
 * "0xcafe" sample failed to compile without them). */
#include "testkit.h"

static unsigned int cafe = 0b1100101011111110;
unsigned int gu;

void main(void)
{
	tk_begin("binlit");

	tk_check_eq(cafe, 0xCAFEu, "static-init");
	gu = 0b101;
	tk_check_eq(gu, 5u, "small");
	gu = 0b1111111111111111;
	tk_check_eq(gu, 0xFFFFu, "allones");
	gu = 0b0;
	tk_check_eq(gu, 0u, "zero");
	tk_check_eq(0b1000 + 0b0111, 15u, "fold");

	tk_end();
}
