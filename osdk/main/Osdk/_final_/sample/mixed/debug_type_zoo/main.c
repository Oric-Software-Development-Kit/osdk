//
// main.c - drives the type-zoo demonstration.
//
// It reads and writes each kind of global, calls library routines (printf /
// sprintf / puts) and crosses into hand-written assembler and back. Good places
// to put a breakpoint: describe(), the AsmXorChecksum() call, and the final loop.
//
#include <stdio.h>
#include "game.h"

// A function with several locals (loop counter, accumulator, a local array and
// a pointer parameter) - gives the debugger a stack frame with locals to show.
void describe(Entity *e)
{
	char line[32];   // local array
	int  total;      // local int (sum of the inventory)
	int  j;          // loop counter

	total = 0;
	for (j = 0; j < INVENTORY; j++)
		total += e->inventory[j];

	sprintf(line, "%s hp=%d loot=%d", e->name, e->hp, total);
	puts(line);
}

// All-locals helper: count entities whose hp is at or above a local threshold.
int count_alive(void)
{
	int i;
	int alive = 0;
	unsigned char threshold = 20;

	for (i = 0; i < MAX_ENTITIES; i++)
		if (g_entities[i].hp >= threshold)
			alive++;

	return alive;
}

void main(void)
{
	int i;

	puts("=== OSDK mixed C/asm type zoo ===");

	// Library call with several printf specifiers.
	printf("score=%d seed=%x xp=%d\n", g_score, g_seed, (int)g_total_xp);

	// Walk the array of structs through the pointer global.
	for (i = 0; i < MAX_ENTITIES; i++) {
		g_current = &g_entities[i];
		describe(g_current);
	}

	// Build the struct-of-arrays from the array-of-structs.
	for (i = 0; i < MAX_ENTITIES; i++) {
		g_world.hp[i]   = g_entities[i].hp;
		g_world.xpos[i] = g_entities[i].x;
		g_world.ypos[i] = g_entities[i].y;
	}

	// Cross into assembler: checksum the SoA hp[] array; the assembler writes
	// the answer back into the C global g_asm_checksum.
	AsmXorChecksum(g_world.hp, MAX_ENTITIES);

	// Bump an assembler-defined counter via asm, then read asm-defined data.
	for (i = 0; i < 5; i++)
		AsmTick();

	printf("checksum=%x palette[3]=%d ticks=%d\n",
	       g_asm_checksum, g_palette[3], g_asm_ticks);

	printf("alive=%d\n", count_alive());

	puts("done - idling (inspect globals now)");

	// Idle forever so every global keeps its final value for inspection.
	for (;;)
		g_frame++;
}
