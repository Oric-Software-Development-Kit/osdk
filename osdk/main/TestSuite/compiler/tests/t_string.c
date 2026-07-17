/* String-function validation for the lib audit batch:
 * strstr, strcspn, strspn, strrchr, memccpy.
 *
 * Each function has a known or suspected defect:
 * - strstr: no haystack rewind after a partial match (misses overlapped
 *   matches), and strstr(s, "") returns garbage.
 * - strcspn/strspn: the s1 page-crossing path jumps back into the INNER
 *   loop, skipping the first character of every page past the first.
 * - strrchr: the returned address adds the scan-advanced high byte, so
 *   any string longer than 256 bytes returns a pointer one page too high;
 *   and strrchr(s, '\0') must return the terminator (C89), not NULL.
 * - memccpy: patched a strncpy opcode byte and jumped to a label that no
 *   longer exists (never linked since the strncpy rewrite); rewritten.
 */
#include "testkit.h"
#include <string.h>

extern void *memccpy(void *dest, const void *src, int c, unsigned int n);

/* Long buffers so the scans cross 6502 page boundaries. */
static char longbuf[301];
static char dst[32];

void main(void)
{
	char *p;
	int n, i;

	tk_begin("string");

	/* ---------------- strstr ---------------------------------------- */
	{
		static char hay1[] = "hello world";
		static char hay2[] = "aaab";
		static char hay3[] = "ababac";
		static char hay4[] = "abc";

		p = strstr(hay1, "world");
		tk_check(p == hay1 + 6, "strstr-basic");
		p = strstr(hay2, "aab");            /* needs the rewind */
		tk_check(p == hay2 + 1, "strstr-rewind");
		p = strstr(hay3, "abac");           /* rewind mid-pattern */
		tk_check(p == hay3 + 2, "strstr-rewind2");
		p = strstr(hay4, "xyz");
		tk_check(p == 0, "strstr-notfound");
		p = strstr(hay4, "");               /* C89: empty needle -> s */
		tk_check(p == hay4, "strstr-empty-needle");
		p = strstr(hay4, "abc");
		tk_check(p == hay4, "strstr-whole");
	}

	/* ---------------- strcspn --------------------------------------- */
	tk_check_eq(strcspn("abcdef", "de"), 3, "strcspn-basic");
	tk_check_eq(strcspn("abc", "xyz"), 3, "strcspn-nostop");

	for (i = 0; i < 300; i++)
		longbuf[i] = 'a';
	longbuf[300] = 0;
	longbuf[270] = 'b';
	tk_check_eq(strcspn(longbuf, "b"), 270, "strcspn-pagecross");
	longbuf[270] = 'a';
	longbuf[256] = 'b';                     /* first char of page 2 */
	tk_check_eq(strcspn(longbuf, "b"), 256, "strcspn-page-edge");
	longbuf[256] = 'a';

	/* ---------------- strspn ---------------------------------------- */
	tk_check_eq(strspn("aaabbb", "a"), 3, "strspn-basic");
	tk_check_eq(strspn("abc", "abc"), 3, "strspn-all");

	longbuf[270] = 'b';
	tk_check_eq(strspn(longbuf, "a"), 270, "strspn-pagecross");
	longbuf[270] = 'a';
	longbuf[256] = 'b';                     /* first char of page 2 */
	tk_check_eq(strspn(longbuf, "a"), 256, "strspn-page-edge");
	longbuf[256] = 'a';

	/* ---------------- strrchr --------------------------------------- */
	{
		static char s6[] = "abcabc";

		p = strrchr(s6, 'b');
		tk_check(p == s6 + 4, "strrchr-last");
		p = strrchr(s6, 'q');
		tk_check(p == 0, "strrchr-notfound");
		p = strrchr(s6, '\0');              /* C89: terminator is found */
		tk_check(p == s6 + 6, "strrchr-nul");
	}
	longbuf[5] = 'z';
	longbuf[280] = 'z';
	p = strrchr(longbuf, 'z');              /* crosses a page boundary */
	tk_check(p == longbuf + 280, "strrchr-pagecross");
	longbuf[5] = 'a';
	longbuf[280] = 'a';

	/* ---------------- memccpy --------------------------------------- */
	{
		static char msrc[] = "hello,world";

		for (i = 0; i < 32; i++)
			dst[i] = 0x7E;
		p = (char *)memccpy(dst, msrc, ',', 20);
		tk_check(p == dst + 6, "memccpy-found-ret");   /* after the ',' */
		n = 1;
		for (i = 0; i < 6; i++)
			if (dst[i] != msrc[i]) n = 0;
		tk_check(n, "memccpy-found-copied");
		tk_check(dst[6] == 0x7E, "memccpy-found-stops"); /* nothing past c */

		for (i = 0; i < 32; i++)
			dst[i] = 0x7E;
		p = (char *)memccpy(dst, msrc, 'q', 5);          /* c not in range */
		tk_check(p == 0, "memccpy-notfound-ret");
		n = 1;
		for (i = 0; i < 5; i++)
			if (dst[i] != msrc[i]) n = 0;
		tk_check(n, "memccpy-notfound-copied");          /* exactly n bytes */
		tk_check(dst[5] == 0x7E, "memccpy-notfound-stops");
	}

	tk_end();
}
