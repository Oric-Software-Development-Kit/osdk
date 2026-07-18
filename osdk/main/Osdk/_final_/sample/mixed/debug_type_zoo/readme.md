OSDK sample: mixed/debug_type_zoo
=================================

A small, self-contained mixed C + assembler program whose only purpose is to
show a source-level debugger every common kind of data it may have to deal with,
all in one standard .tap (no FloppyBuilder, no Sedoric, no overlays).

Build:   osdk_build.bat      (produces BUILD\DBGZOO.tap)
Run:     osdk_execute.bat

Files
-----
  game.h         Types shared by the C files: an enum, a struct with mixed
                 fields (embedded string, enum, ints, unsigned char, embedded
                 array), and a "struct of arrays". Also the extern declarations
                 for every cross-module symbol.
  world.c        The C-defined global data: an array of structs (AoS), a struct
                 of arrays (SoA), a pointer global, and scalars of several
                 widths (unsigned char / int / unsigned int / long).
  main.c         Program logic. Functions with local variables (a local array,
                 loop counters, accumulators), library calls (printf / sprintf /
                 puts), and calls across the C <-> asm boundary.
  asm_data.s     Data DEFINED IN ASSEMBLER and read from C (g_palette[],
                 g_asm_ticks).
  asm_helpers.s  Routines called FROM C (AsmXorChecksum, AsmTick). One of them
                 writes a C-defined global (g_asm_checksum); the other bumps an
                 asm-defined global.

What to inspect in the debugger
-------------------------------
  * g_entities   - array of structs; expand an element to see nested members
                   and the embedded name[] / inventory[] arrays.
  * g_world      - struct of arrays; compare its layout against g_entities.
  * g_current    - pointer global; follow it to the entity it points at.
  * g_score / g_seed / g_total_xp / g_frame - scalars of different widths.
  * g_palette / g_asm_ticks - globals that live in assembler but are read by C.
  * g_asm_checksum - a C global written by hand-written assembler.
  * describe()   - break here to see a live stack frame with locals (line[],
                   total, j) and the pointer parameter.

Optimization level and locals
-----------------------------
The project builds at -O2 (see osdk_config.bat), but at that level the compiler
promotes busy locals to zero page virtual registers and can omit the stack
frame, hiding them from the debugger. main.c therefore wraps describe() and
count_alive() in #pragma optimize(push, 1) / #pragma optimize(pop) so exactly
those two functions keep debugger-visible stack locals while everything else
stays fully optimized (see the Compiler documentation).

Cross-language symbol naming
----------------------------
A C name Foo (function or global) is the assembler label _Foo. The linker
matches the two, so declaring "extern unsigned int g_asm_ticks;" in C resolves
to the "_g_asm_ticks" label in asm_data.s, and calling AsmTick() in C jumps to
"_AsmTick" in asm_helpers.s.
