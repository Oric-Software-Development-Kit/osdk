/* MacroSplitter -O temp liveness: a byte staged in a compiler temp stays live
 * across a short-circuit branch. The optimiser used to judge temp deadness with
 * a straight-line scan that stopped at the first branch and called the temp
 * dead, so in `if ((t & a) || ((t & b) == c))` it dropped the reload in front of
 * the second test and left that test reading the FIRST test's result. Nothing
 * warned; the program just answered wrong. Byte-typed locals are what expose it
 * (the word path reloads through CSBW and overwrites the stale value first), so
 * every predicate here is built on unsigned char.
 *
 * The shapes come from Oric DungeonMaster's move_party, where an unsigned char
 * cell type made every open door read as shut. See
 * docs/macrosplitter-stale-tmp-bug.md. Run with -Peephole to exercise it. */
#include "testkit.h"

#define GROUND_SOLIDE   1
#define GROUND_CHUTE    4
#define GROUND_CLOSABLE 8
#define GROUND_STAIRS   16
#define GROUND_OPEN     128

/* global source: the local's value is not a constant the compiler can fold */
unsigned char gCellType;
static unsigned int stairs_calls;

static unsigned char Stairs(void) { stairs_calls++; return 0; }

/* The reported shape: a join point (the if/call above), then two tests of one
 * byte local either side of the || short-circuit branch. */
static unsigned char blocked(void)
{
	unsigned char type;
	type = gCellType;
	if (type & GROUND_STAIRS) {
		if (Stairs()) return 0;
	}
	if ((type & (GROUND_SOLIDE | GROUND_CHUTE))
	    || ((type & (GROUND_CLOSABLE | GROUND_OPEN)) == GROUND_CLOSABLE))
		return 1;
	return 0;
}

/* The same value read either side of an && branch: a shut door is closable
 * and not open. */
static unsigned char shut_door(void)
{
	unsigned char type;
	type = gCellType;
	if (type & GROUND_STAIRS) {
		if (Stairs()) return 0;
	}
	if ((type & GROUND_CLOSABLE) && !(type & GROUND_OPEN))
		return 1;
	return 0;
}

/* Three reads of one byte across two branches, so a fold that rewrites only
 * the first read is caught even when the second one happens to agree. */
static unsigned char three_way(void)
{
	unsigned char t;
	t = gCellType;
	if (t & GROUND_STAIRS) {
		if (Stairs()) return 0;
	}
	if (t & GROUND_SOLIDE)  return 1;
	if (t & GROUND_CHUTE)   return 2;
	if (t & GROUND_CLOSABLE) return 3;
	return 0;
}

/* The word-typed twin of blocked(): unaffected by the bug, here to prove the
 * fix costs the word path nothing. */
static unsigned int blocked_word(void)
{
	unsigned int type;
	type = gCellType;
	if (type & GROUND_STAIRS) {
		if (Stairs()) return 0;
	}
	if ((type & (GROUND_SOLIDE | GROUND_CHUTE))
	    || ((type & (GROUND_CLOSABLE | GROUND_OPEN)) == GROUND_CLOSABLE))
		return 1;
	return 0;
}

/* A byte staged for a commutative op, then read again after a branch: the
 * `lda X : sta T : lda Y : and T` fold must keep T when the later test needs
 * it. */
static unsigned char masked_twice(unsigned char mask)
{
	unsigned char t;
	t = gCellType & mask;
	if (t == 0) return 0;
	if (t & GROUND_OPEN) return 1;
	return 2;
}

void main(void)
{
	tk_begin("peephole");

	/* --- blocked(): the door predicate that broke --- */
	gCellType = GROUND_CLOSABLE;                 /* shut door */
	tk_check_eq(blocked(), 1, "blocked-shut-door");
	gCellType = GROUND_CLOSABLE | GROUND_OPEN;   /* open door */
	tk_check_eq(blocked(), 0, "blocked-open-door");
	gCellType = GROUND_SOLIDE;
	tk_check_eq(blocked(), 1, "blocked-solid");
	gCellType = GROUND_CHUTE;
	tk_check_eq(blocked(), 1, "blocked-chute");
	gCellType = 0;
	tk_check_eq(blocked(), 0, "blocked-empty");
	gCellType = GROUND_OPEN;                     /* open bit alone: not a door */
	tk_check_eq(blocked(), 0, "blocked-open-bit-only");

	/* the same, past the stairs join point */
	stairs_calls = 0;
	gCellType = GROUND_STAIRS;
	tk_check_eq(blocked(), 0, "blocked-stairs");
	gCellType = GROUND_STAIRS | GROUND_CLOSABLE;
	tk_check_eq(blocked(), 1, "blocked-stairs-shut-door");
	gCellType = GROUND_STAIRS | GROUND_CLOSABLE | GROUND_OPEN;
	tk_check_eq(blocked(), 0, "blocked-stairs-open-door");
	tk_check_eq(stairs_calls, 3, "blocked-stairs-called");

	/* --- shut_door(): the && form --- */
	gCellType = GROUND_CLOSABLE;
	tk_check_eq(shut_door(), 1, "shut-closable-not-open");
	gCellType = GROUND_CLOSABLE | GROUND_OPEN;
	tk_check_eq(shut_door(), 0, "shut-closable-and-open");
	gCellType = GROUND_OPEN;
	tk_check_eq(shut_door(), 0, "shut-open-not-closable");
	gCellType = GROUND_STAIRS | GROUND_CLOSABLE;
	tk_check_eq(shut_door(), 1, "shut-past-join-point");

	/* --- three_way(): first match wins, later reads still valid --- */
	gCellType = GROUND_SOLIDE | GROUND_CHUTE;
	tk_check_eq(three_way(), 1, "three-way-solid-first");
	gCellType = GROUND_CHUTE | GROUND_CLOSABLE;
	tk_check_eq(three_way(), 2, "three-way-chute");
	gCellType = GROUND_CLOSABLE;
	tk_check_eq(three_way(), 3, "three-way-closable");
	gCellType = GROUND_STAIRS | GROUND_CLOSABLE;
	tk_check_eq(three_way(), 3, "three-way-past-join-point");
	gCellType = GROUND_OPEN;
	tk_check_eq(three_way(), 0, "three-way-none");

	/* --- the word twin --- */
	gCellType = GROUND_CLOSABLE;
	tk_check_eq(blocked_word(), 1, "word-shut-door");
	gCellType = GROUND_CLOSABLE | GROUND_OPEN;
	tk_check_eq(blocked_word(), 0, "word-open-door");

	/* --- staged mask read twice --- */
	gCellType = GROUND_CLOSABLE | GROUND_OPEN;
	tk_check_eq(masked_twice(GROUND_OPEN), 1, "masked-open");
	tk_check_eq(masked_twice(GROUND_CLOSABLE), 2, "masked-closable");
	tk_check_eq(masked_twice(GROUND_SOLIDE), 0, "masked-none");

	tk_end();
}
