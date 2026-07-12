//
// world.c - the C-side global data for the type-zoo sample.
//
// Everything here is a global so it lands at a fixed, named address and is easy
// to find in the debugger's symbol list.
//
#include "game.h"

// Array of structs, statically initialised (AoS).
Entity g_entities[MAX_ENTITIES] = {
	{ "Hero",  KIND_HERO,   10, 20, 100, { 1, 2, 3 } },
	{ "Grik",  KIND_GOBLIN,  5,  8,  15, { 4, 0, 0 } },
	{ "Grok",  KIND_GOBLIN,  7,  9,  15, { 0, 5, 0 } },
	{ "Smaug", KIND_DRAGON, 30, 30, 255, { 9, 9, 9 } }
};

// Struct of arrays (filled at run time from g_entities in main()).
World g_world;

// A pointer global, initialised to the first entity.
Entity *g_current = &g_entities[0];

// Scalar globals of assorted widths - each is easy to spot in the debugger.
unsigned char g_frame     = 0;
int           g_score     = 1234;
unsigned int  g_seed      = 0xACCA;
long          g_total_xp  = 100000;

// A C-defined byte that the ASSEMBLER writes into (see AsmXorChecksum).
unsigned char g_asm_checksum = 0;
