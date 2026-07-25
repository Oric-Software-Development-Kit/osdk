/* __fastcall register parameter passing: exercise every argument addressing
 * mode through a register-passing library function, so a missing ARGR macro
 * (the ARGRW_Y / ARGRW_A gaps that broke aes256 -O3 and _puts(localbuf))
 * is caught here instead of only by the benchmark.
 *   - atoi(char*) covers the WORD (A:X) modes and is value-checkable.
 *   - ctype(char)  covers the BYTE (A) modes.
 * Taking &param / &local keeps the operand frame-resident, which is what
 * makes the compiler emit the Y (indirect) and A (frame-address) forms
 * instead of promoting to a temp (mode D). */
#include "testkit.h"
#include <ctype.h>
#include <stdlib.h>

char gdig  = '7';
char *gstr = "42";

/* char argument read from a frame-resident parameter -> mode Y */
int digit_of(char c) { char *k = &c; *k = c; return isdigit(c); }
int upper_of(char c) { char *k = &c; *k = c; return toupper(c); }

/* char* argument from a frame-resident parameter -> mode Y */
int atoi_param(char *s) { char **k = &s; *k = s; return atoi(s); }

void main(void)
{
	char local[6];

	tk_begin("fastcall");

	/* ARGRB - char arg. C = immediate, D = global, Y = param */
	tk_check_eq(isdigit('5') != 0, 1, "argrb_c");
	tk_check_eq(isdigit('x') != 0, 0, "argrb_c_neg");
	tk_check_eq(isdigit(gdig) != 0, 1, "argrb_d");
	tk_check_eq(digit_of('9') != 0, 1, "argrb_y");
	tk_check_eq(upper_of('a'), 'A', "argrb_y_ret");

	/* ARGRW - char* arg -> A:X. C = literal, D = global, Y = param, A = local */
	tk_check_eq(atoi("123"), 123, "argrw_c");
	tk_check_eq(atoi(gstr), 42, "argrw_d");
	tk_check_eq(atoi_param("-7"), -7, "argrw_y");
	local[0] = '9'; local[1] = '9'; local[2] = 0;
	tk_check_eq(atoi(local), 99, "argrw_a");

	tk_end();
}
