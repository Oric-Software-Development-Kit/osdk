//
// game.h - shared declarations for the OSDK "mixed" debugging type-zoo sample.
//
// This sample exists to give a source-level debugger something rich to inspect:
// every common C data shape (scalars of several widths, arrays, a struct, an
// array of structs, a struct of arrays, pointers) plus symbols that cross the
// C <-> assembler boundary in BOTH directions.
//
// Note: the OSDK C build defines __NOFLOAT__, so there is no floating point here.
//
#ifndef GAME_H
#define GAME_H

#define MAX_ENTITIES 4
#define NAME_LEN     8
#define INVENTORY    3

// A plain (sequential) enum: the debugger maps the stored value back to its name.
typedef enum {
	KIND_HERO   = 0,
	KIND_GOBLIN = 1,
	KIND_DRAGON = 2
} EntityKind;

// A bit-flag enum: each member is a distinct power-of-two bit, meant to be OR'd
// together to pack several booleans into one byte (the classic 8-bit space-saver).
// The debugger auto-detects the single-bit layout and decomposes a stored value
// back into "A|B|C" instead of leaving you to check each bit by hand.
typedef enum {
	STATUS_NONE     = 0,
	STATUS_AWAKE    = 1,
	STATUS_POISONED = 2,
	STATUS_ARMED    = 4,
	STATUS_BOSS     = 8
} EntityStatus;

// A struct mixing an embedded char array, an enum, signed ints, an unsigned
// char and an embedded int array. Inspecting one exercises nested members and
// mixed field widths.
typedef struct {
	char          name[NAME_LEN];       // embedded string
	EntityKind    kind;                 // enum field
	int           x;                    // signed 16-bit
	int           y;
	unsigned char hp;                   // unsigned 8-bit
	int           inventory[INVENTORY]; // embedded array (a "struct of arrays" in miniature)
} Entity;

// The same data laid out as a "struct of arrays" (SoA) - nice to compare against
// the array-of-structs g_entities in the debugger's variable view.
typedef struct {
	unsigned char hp[MAX_ENTITIES];
	int           xpos[MAX_ENTITIES];
	int           ypos[MAX_ENTITIES];
} World;

// ---- Globals DEFINED IN C (see world.c) ------------------------------------
extern Entity        g_entities[MAX_ENTITIES];  // array of structs
extern World         g_world;                   // struct of arrays
extern Entity       *g_current;                 // pointer into g_entities
extern unsigned char g_frame;                   // 8-bit
extern int           g_score;                   // signed 16-bit
extern unsigned int  g_seed;                    // unsigned 16-bit
extern long          g_total_xp;                // 32-bit
extern EntityStatus  g_hero_status;             // bit-flags OR'd together
extern unsigned char g_asm_checksum;            // written by the assembler

// ---- Globals DEFINED IN ASSEMBLER (see asm_data.s) -------------------------
extern unsigned char g_palette[8];   // a byte table built in asm
extern unsigned int  g_asm_ticks;    // a 16-bit counter bumped by asm

// ---- Routines IMPLEMENTED IN ASSEMBLER (see asm_helpers.s) -----------------

// XOR every byte of 'data' (count bytes, count < 256) and store the result in
// the C global g_asm_checksum. Demonstrates the assembler reading a C pointer
// parameter and writing a C-defined global.
void AsmXorChecksum(unsigned char *data, int count);

// Increment the assembler-defined 16-bit counter g_asm_ticks by one.
void AsmTick(void);

#endif // GAME_H
