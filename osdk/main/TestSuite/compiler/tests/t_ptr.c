/* Pointers, arrays, multi-dimensional indexing (the OSDK 1.14 array bug
 * test case) and the 2019 forum -O3 static-pointer copy loop. */
#include "testkit.h"

unsigned char grid[3][4][5];
unsigned char src_data[8] = { 11, 22, 33, 44, 55, 66, 77, 0 };
unsigned char dst_data[8];

static unsigned char *from_ptr = src_data;
static unsigned char *to_ptr   = dst_data;

int gi;
int giarr[10];
char *gcp;
char **gcpp;
char gc;

/* the exact shape of the 2019 "Better code generation" forum bug */
void copy_font(void)
{
	while (*from_ptr) {
		*to_ptr++ = *from_ptr++;
	}
}

/* Return-position variant of the ptr-arith deref: at -O3 this folds
 * into RETW_Y, unlike the argument-position variant which inserts a
 * CVIU conversion (the shape of the 2026-07 elided-dereference bug). */
int deref_ret(void)
{
	return *(giarr + 2 + gi);
}

void main(void)
{
	int s, l, c, n;

	tk_begin("ptr");

	/* multi-dimensional array indexing, constant and variable */
	n = 0;
	for (s = 0; s < 3; s++)
		for (l = 0; l < 4; l++)
			for (c = 0; c < 5; c++)
				grid[s][l][c] = (unsigned char)n++;
	tk_check_eq(grid[0][0][0], 0, "md-first");
	tk_check_eq(grid[1][2][3], 33, "md-mid");       /* 20+10+3 */
	tk_check_eq(grid[2][3][4], 59, "md-last");
	s = 1; l = 2; c = 3;
	tk_check_eq(grid[s][l][c], 33, "md-var");

	/* pointer walk over the same array */
	{
		unsigned char *p = &grid[0][0][0];
		unsigned int sum = 0;
		for (n = 0; n < 60; n++)
			sum += *p++;
		tk_check_eq(sum, 1770u, "walk-sum");        /* 0+1+..+59 */
	}

	/* static-pointer copy loop (2019 -O3 bug shape) */
	copy_font();
	tk_check_eq(dst_data[0], 11, "font-0");
	tk_check_eq(dst_data[6], 77, "font-6");
	tk_check_eq(dst_data[7], 0, "font-end");

	/* pointer arithmetic and double indirection */
	gi = 3;
	giarr[5] = 0x1234;
	tk_check_eq(*(giarr + 2 + gi), 0x1234u, "ptr-arith");   /* arg position (CVIU) */
	tk_check_eq(deref_ret(), 0x1234u, "ptr-arith-ret");     /* return position */
	gc = 'Q';
	gcp = &gc;
	gcpp = &gcp;
	tk_check_eq(**gcpp, 'Q', "double-deref");
	**gcpp = 'R';
	tk_check_eq(gc, 'R', "double-deref-store");

	tk_end();
}
