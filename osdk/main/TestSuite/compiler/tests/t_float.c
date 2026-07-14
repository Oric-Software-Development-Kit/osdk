/* Floating point: broken between Compiler 1.40 and the 1.41 fix
 * (all float temporaries were marked busy, any float expression died
 * with "expression too complex"). */
#include "testkit.h"

float fa, fb, fc;
int gi;

float scale(float x) { return x * 2.0 + 1.0; }

void main(void)
{
	tk_begin("float");

	fa = 1.5; fb = 2.25;
	fc = fa + fb; tk_check(fc == 3.75, "add");
	fc = fb - fa; tk_check(fc == 0.75, "sub");
	fc = fa * fb; tk_check(fc == 3.375, "mul");
	fc = fb / fa; tk_check(fc == 1.5, "div");
	fc = -fa;     tk_check(fc == -1.5, "neg");

	tk_check(fa < fb,  "lt");
	tk_check(fb > fa,  "gt");
	tk_check(fa != fb, "ne");

	gi = (int)(fa * 4.0);
	tk_check_eq(gi, 6, "ftoi");
	fc = (float)10;
	tk_check(fc == 10.0, "itof");
	gi = -3;
	fc = (float)gi;
	tk_check(fc == -3.0, "itof-neg");

	fc = scale(fa);
	tk_check(fc == 4.0, "call");

	tk_end();
}
