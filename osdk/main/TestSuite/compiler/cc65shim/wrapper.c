/*
 * wrapper.c - runs one UNMODIFIED cc65 test/val file on the OSDK runtime.
 *
 * The test file is included with its main() renamed to test_main(). Before
 * calling it, the ROM character-output vector at $0238 (a jmp instruction
 * in RAM, used by the OSDK printf) is repointed to the ROM printer routine
 * PrintChar ($F5C1), so everything the test prints lands in Oricutron's
 * printer_out.txt where the runner can read it.
 *
 * Protocol appended after the test's own output:
 *   @RESULT failures=<hex return value> ticks=<hex 100Hz ticks>
 *   @END
 */
#include "testkit.h"

#define main test_main
#include "testcase.c"
#undef main

int main(void)
{
	unsigned int r, t;

	/* $0238: jmp $xxxx - repoint to PrintChar ($F5C1) */
	*(unsigned char*)0x0239 = 0xC1;
	*(unsigned char*)0x023A = 0xF5;

	tk_timer_reset();
	r = (unsigned int)test_main();
	t = tk_timer_read();

	tk_puts("\n@RESULT failures=");
	tk_puthex(r);
	tk_puts(" ticks=");
	tk_puthex(t);
	tk_putc('\n');
	tk_puts("@END\n");
	for (;;)
		;
}
