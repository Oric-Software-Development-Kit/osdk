/*
 * benchmain.c - runs one UNMODIFIED ISS oricCompilerBenchmark sample.
 *
 * The sample's own output goes through _putc (cport.s), a plain store,
 * so it does not distort the timing the way the ROM printer path would.
 * The result protocol is printed through the emulated printer AFTER the
 * timer is read:
 *   @RESULT failures=<hex return value> ticks=<hex 100Hz ticks>
 *   @END
 */
#include "testkit.h"

/* The sample keeps its original directory depth (scaffold/sample/) so its
   own relative includes like "../sort-helper.h" resolve as they do in the
   benchmark tree. */
#define main bench_main
#include "sample/testcase.c"
#undef main

int main(void)
{
	int r;
	unsigned int t;

	/* $0238: jmp $xxxx - repoint to PrintChar ($F5C1) for the protocol */
	*(unsigned char*)0x0239 = 0xC1;
	*(unsigned char*)0x023A = 0xF5;

	/* $FFFE: cport cycle counter start/stop - a ROM-space write, so a
	   no-op on real hardware; a --cport emulator reports exact cycles */
	tk_timer_reset();
	*(unsigned char*)0xFFFE = 1;
	r = bench_main();
	*(unsigned char*)0xFFFE = 0;
	t = tk_timer_read();

	tk_puts("\n@RESULT failures=");
	tk_puthex((unsigned int)r);
	tk_puts(" ticks=");
	tk_puthex(t);
	tk_putc('\n');
	tk_puts("@END\n");
	for (;;)
		;
	return 0;
}
