/* printf/sprintf conversions: %d %u %x %c %s, including values above
 * 32767 (found empty %u output while importing cc65 compare-rev13). */
#include "testkit.h"
#include <stdio.h>

char buf[40];
unsigned int gu;
int gi;

static void chk(const char *want, const char *name)
{
	int i;
	for (i = 0; want[i] || buf[i]; i++)
		if (want[i] != buf[i]) {
			tk_fail++;
			tk_puts("FAIL ");
			tk_puts(name);
			tk_puts(" got=");
			tk_puts(buf);
			tk_puts(" want=");
			tk_puts(want);
			tk_putc('\n');
			return;
		}
	tk_pass++;
}

void main(void)
{
	tk_begin("printf");

	gi = 1234;
	sprintf(buf, "%d", gi);      chk("1234", "d-pos");
	gi = -1234;
	sprintf(buf, "%d", gi);      chk("-1234", "d-neg");
	gi = 0;
	sprintf(buf, "%d", gi);      chk("0", "d-zero");
	gi = -32768;
	sprintf(buf, "%d", gi);      chk("-32768", "d-min");

	gu = 7u;
	sprintf(buf, "%u", gu);      chk("7", "u-small");
	gu = 32767u;
	sprintf(buf, "%u", gu);      chk("32767", "u-mid");
	gu = 32768u;
	sprintf(buf, "%u", gu);      chk("32768", "u-msb");
	gu = 45866u;
	sprintf(buf, "%u", gu);      chk("45866", "u-big");
	gu = 65535u;
	sprintf(buf, "%u", gu);      chk("65535", "u-max");

	gu = 0xB32Au;
	sprintf(buf, "%x", gu);      chk("B32A", "x");
	gu = 0x0007u;
	sprintf(buf, "%x", gu);      chk("0007", "x-lead");

	sprintf(buf, "%c%c", 'O', 'K');       chk("OK", "c");
	sprintf(buf, "[%s]", "oric");         chk("[oric]", "s");
	gi = 42;
	sprintf(buf, "a=%d b=%u!", gi, 7u);   chk("a=42 b=7!", "mixed");

	tk_end();
}
