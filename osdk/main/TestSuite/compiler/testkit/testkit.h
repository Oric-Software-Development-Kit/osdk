/*
 * testkit.h - minimal self-checking test support for the OSDK compiler
 * test suite.
 *
 * Results are sent to the emulated printer (Oricutron writes them to
 * printer_out.txt), so an external runner can parse them without looking
 * at the screen. Protocol, one line each:
 *
 *   @TEST <name>
 *   FAIL <check-name>        (zero or more, only for failing checks)
 *   @RESULT pass=<hex> fail=<hex> ticks=<hex>
 *   @END
 *
 * Timing uses the ROM 100Hz countdown timer at $0272/$0273, so 'ticks'
 * are hundredths of a second spent between tk_begin() and tk_end().
 */

extern void tk_putc(int c);
extern void tk_timer_reset(void);
extern unsigned int tk_timer_read(void);

static unsigned int tk_pass = 0;
static unsigned int tk_fail = 0;

static void tk_puts(const char *s)
{
	while (*s)
		tk_putc(*s++);
}

static void tk_puthex(unsigned int v)
{
	static const char digits[] = "0123456789ABCDEF";
	tk_putc(digits[(v >> 12) & 15]);
	tk_putc(digits[(v >>  8) & 15]);
	tk_putc(digits[(v >>  4) & 15]);
	tk_putc(digits[ v        & 15]);
}

static void tk_check(int cond, const char *name)
{
	if (cond)
		tk_pass++;
	else {
		tk_fail++;
		tk_puts("FAIL ");
		tk_puts(name);
		tk_putc('\n');
	}
}

/* Check with the offending value printed, to ease diagnosis */
static void tk_check_eq(unsigned int value, unsigned int expected, const char *name)
{
	if (value == expected)
		tk_pass++;
	else {
		tk_fail++;
		tk_puts("FAIL ");
		tk_puts(name);
		tk_puts(" got=");
		tk_puthex(value);
		tk_puts(" want=");
		tk_puthex(expected);
		tk_putc('\n');
	}
}

static void tk_begin(const char *name)
{
	tk_puts("@TEST ");
	tk_puts(name);
	tk_putc('\n');
	tk_timer_reset();
}

static void tk_end(void)
{
	unsigned int t = tk_timer_read();
	tk_puts("@RESULT pass=");
	tk_puthex(tk_pass);
	tk_puts(" fail=");
	tk_puthex(tk_fail);
	tk_puts(" ticks=");
	tk_puthex(t);
	tk_putc('\n');
	tk_puts("@END\n");
	for (;;)
		; /* idle forever; the runner kills the emulator */
}
