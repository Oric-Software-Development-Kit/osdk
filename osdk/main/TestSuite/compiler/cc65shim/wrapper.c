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

/* Capture exit(): the OSDK exit unwinds straight back to BASIC, which
 * would swallow the result protocol - tests aborting through assert
 * macros (cc65's unittest.h calls exit(EXIT_FAILURE)) would look like
 * timeouts. The macro renames every exit reference in the test source
 * (including the declaration it gets from stdlib.h) to our reporter. */
static void tk_exit(int code);

/* C89 emulation of the C11 _Static_assert keyword: a false condition
 * makes the typedef'd array size negative, which is a (compile time)
 * constraint violation - honest semantics, just a worse error message.
 * Variadic so the C23 one-argument form _Static_assert(cond) also works
 * (mcpp only warns about a missing variadic argument; it errors on a
 * plain two-parameter macro called with one argument). */
#define tk_sa_paste2(a,b) a##b
#define tk_sa_paste(a,b) tk_sa_paste2(a,b)
#define _Static_assert(cond, ...) \
	typedef char tk_sa_paste(tk_static_assert_, __LINE__)[(cond) ? 1 : -1]

#define main test_main
#define exit tk_exit
#include "testcase.c"
#undef exit
#undef main

static void tk_exit(int code)
{
	unsigned int t = tk_timer_read();
	tk_puts("\n@RESULT failures=");
	tk_puthex((unsigned int)code);
	tk_puts(" ticks=");
	tk_puthex(t);
	tk_putc('\n');
	tk_puts("@END\n");
	for (;;)
		;
}

int main(void)
{
	/* $0238: jmp $xxxx - repoint to PrintChar ($F5C1) */
	*(unsigned char*)0x0239 = 0xC1;
	*(unsigned char*)0x023A = 0xF5;

	tk_timer_reset();
	tk_exit((int)test_main());
	return 0;
}
