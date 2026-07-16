/* Dynamic memory allocation API validation: malloc / free / realloc / strdup.
 *
 * This subsystem is rarely exercised (most OSDK code uses static allocation),
 * so this test drives it hard and checks actual behaviour: allocation within
 * the heap bounds, no overlap, heap accounting (nheapbytes/nheapdesc), reuse
 * of freed blocks, realloc grow/shrink/NULL/zero with data preservation, the
 * heap-end bound (malloc must return NULL rather than overrun), and strdup
 * content correctness.
 */
#include "testkit.h"

extern void *malloc(unsigned int n);
extern void  free(void *p);
extern void *realloc(void *p, unsigned int n);
extern char *strdup(char *s);

/* Heap accounting, exported by malloc.s (asm _name -> C name). */
extern unsigned int  nheapbytes;   /* total bytes (incl. per-block overhead) */
extern unsigned int  nheapdesc;    /* number of live descriptors            */
extern unsigned int  heapstart;    /* first heap address                    */
extern unsigned int  heapend;      /* one past the last heap address        */
extern unsigned int  heapsize;     /* heapend - heapstart                   */
extern unsigned char heapovh;      /* per-block descriptor overhead (=4)     */

static int in_heap(void *p, unsigned int n)
{
	unsigned int a = (unsigned int)p;
	if (a < heapstart) return 0;
	if (a + n > heapend) return 0;
	return 1;
}

static int mem_all(unsigned char *p, unsigned int n, unsigned char v)
{
	unsigned int i;
	for (i = 0; i < n; i++)
		if (p[i] != v) return 0;
	return 1;
}

static void mem_fill(unsigned char *p, unsigned int n, unsigned char v)
{
	unsigned int i;
	for (i = 0; i < n; i++)
		p[i] = v;
}

void main(void)
{
	unsigned char *p1, *p2, *p3;
	unsigned char *g, *s;
	unsigned int nb, nd, ov;

	tk_begin("malloc");

	ov = heapovh;

	/* --- basic malloc ------------------------------------------------ */
	p1 = (unsigned char *)malloc(16);
	tk_check(p1 != 0, "malloc-nonnull");
	tk_check(in_heap(p1, 16), "malloc-in-heap");
	mem_fill(p1, 16, 0xAA);
	tk_check(mem_all(p1, 16, 0xAA), "malloc-writable");

	/* accounting: one block of 16+overhead */
	tk_check_eq(nheapdesc, 1, "acct-desc-1");
	tk_check_eq(nheapbytes, 16 + ov, "acct-bytes-1");

	/* --- second allocation must not overlap the first ---------------- */
	nb = nheapbytes;
	nd = nheapdesc;
	p2 = (unsigned char *)malloc(16);
	tk_check(p2 != 0, "malloc2-nonnull");
	tk_check(p2 != p1, "malloc2-distinct");
	tk_check_eq(nheapdesc - nd, 1, "acct-desc-2");
	tk_check_eq(nheapbytes - nb, 16 + ov, "acct-bytes-2");
	mem_fill(p2, 16, 0x55);
	tk_check(mem_all(p1, 16, 0xAA), "no-overlap-p1");   /* p1 untouched */
	tk_check(mem_all(p2, 16, 0x55), "no-overlap-p2");

	/* --- free then re-allocate: freed slot should be reused ---------- */
	free(p1);
	tk_check_eq(nheapdesc, 1, "free-desc-dec");
	tk_check_eq(nheapbytes, 16 + ov, "free-bytes-dec");
	p3 = (unsigned char *)malloc(16);
	tk_check(p3 != 0, "remalloc-nonnull");
	tk_check(p3 == p1, "remalloc-reuses-slot");
	free(p3);
	free(p2);
	tk_check_eq(nheapdesc, 0, "free-all-desc");
	tk_check_eq(nheapbytes, 0, "free-all-bytes");

	/* --- realloc: grow, preserving data ------------------------------ */
	p1 = (unsigned char *)malloc(8);
	mem_fill(p1, 8, 0x3C);
	p2 = (unsigned char *)realloc(p1, 40);
	tk_check(p2 != 0, "realloc-grow-nonnull");
	tk_check(in_heap(p2, 40), "realloc-grow-in-heap");
	tk_check(mem_all(p2, 8, 0x3C), "realloc-grow-preserved");
	tk_check_eq(nheapbytes, 40 + ov, "realloc-grow-acct");
	tk_check_eq(nheapdesc, 1, "realloc-grow-desc");
	mem_fill(p2, 40, 0x11);
	tk_check(mem_all(p2, 40, 0x11), "realloc-grow-writable");

	/* --- realloc: shrink in place, preserving data ------------------- */
	p3 = (unsigned char *)realloc(p2, 12);
	tk_check(p3 != 0, "realloc-shrink-nonnull");
	tk_check(mem_all(p3, 12, 0x11), "realloc-shrink-preserved");
	tk_check_eq(nheapbytes, 12 + ov, "realloc-shrink-acct");
	tk_check_eq(nheapdesc, 1, "realloc-shrink-desc");
	free(p3);
	tk_check_eq(nheapdesc, 0, "realloc-cleanup");

	/* --- realloc(NULL, n) behaves as malloc -------------------------- */
	p1 = (unsigned char *)realloc((void *)0, 24);
	tk_check(p1 != 0, "realloc-null-is-malloc");
	tk_check(in_heap(p1, 24), "realloc-null-in-heap");

	/* --- realloc(p, 0) behaves as free ------------------------------- */
	nd = nheapdesc;
	realloc(p1, 0);
	tk_check_eq(nd - nheapdesc, 1, "realloc-zero-is-free");

	/* --- strdup ------------------------------------------------------ */
	tk_check_eq(nheapdesc, 0, "pre-strdup-clean");
	s = (unsigned char *)strdup("hello");
	tk_check(s != 0, "strdup-nonnull");
	tk_check(in_heap(s, 6), "strdup-in-heap");
	tk_check(s[0] == 'h' && s[1] == 'e' && s[2] == 'l' &&
	         s[3] == 'l' && s[4] == 'o' && s[5] == 0, "strdup-contents");
	tk_check_eq(nheapdesc, 1, "strdup-one-block");
	free(s);
	tk_check_eq(nheapdesc, 0, "strdup-freed");

	/* --- heap-end bound: an over-large request must fail, not overrun */
	/* heapsize is set on first malloc; ask for the whole heap (which
	 * cannot fit once per-block overhead is added).                    */
	p1 = (unsigned char *)malloc(heapsize);
	tk_check(p1 == 0, "malloc-oom-bounded");
	tk_check_eq(nheapdesc, 0, "oom-no-leak");

	tk_end();
}
