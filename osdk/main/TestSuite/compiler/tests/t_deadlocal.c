/* Dead-local elimination: a write-only auto local loses its stack slot (and,
 * if it was the sole frame-forcing feature, the whole ENTER/LEAVE frame), but
 * the right-hand side of its store must still be evaluated for its side
 * effects. This checks the observable behaviour is unchanged: side-effecting
 * RHS still runs, pure RHS produces nothing, and locals that ARE read (incl.
 * via a taken address) keep their value. See dead-local-elimination brief. */
#include "testkit.h"

static unsigned int counter;

/* side effect: bumps a global and returns its argument */
static int side(int v) { counter++; return v; }

/* dead local from a side-effecting call: the call must still fire */
static void dead_call(void)
{
	int a;
	a = side(42);           /* a never read - store dropped, side() kept */
}

/* dead local from a pure expression: nothing observable must happen */
static void dead_pure(void)
{
	int a;
	a = counter + 999;      /* a never read, no side effect - vanishes */
}

/* first store dead, second local read: only the first is eliminated */
static int dead_then_live(void)
{
	int a, b;
	a = side(7);            /* dead */
	b = side(9);            /* live (returned) */
	return b;
}

/* x = a = f(): a is dead, but the value must still reach the global */
static unsigned int chain_target;
static void chain(void)
{
	int a;
	chain_target = a = side(123);
}

/* address taken: a is written then read only through a pointer - must keep */
static int addr_taken(void)
{
	int a;
	int *p = &a;
	a = side(55);
	return *p;
}

void main(void)
{
	tk_begin("deadlocal");

	counter = 0;
	dead_call();
	tk_check_eq(counter, 1, "dead-call-side-effect");

	counter = 5;
	dead_pure();
	tk_check_eq(counter, 5, "dead-pure-no-effect");

	counter = 0;
	tk_check_eq(dead_then_live(), 9, "dead-then-live-ret");
	tk_check_eq(counter, 2, "dead-then-live-both-ran");

	counter = 0;
	chain();
	tk_check_eq(chain_target, 123, "chain-value");
	tk_check_eq(counter, 1, "chain-side-effect");

	counter = 0;
	tk_check_eq(addr_taken(), 55, "addr-taken-through-ptr");
	tk_check_eq(counter, 1, "addr-taken-side-effect");

	tk_end();
}
