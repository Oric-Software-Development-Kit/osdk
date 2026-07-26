
/*
Change history for the Compiler (6502 backend)

1.35
- defstring is used by the frontend not only to define strings (signed char
  data) but also for initializing unsigned char arrays. In this case,
  defstring will receive char data with possible negative char values which
  have to be printed as their corresponding unsigned char value.

1.36
- Fixes for the -O0 output

1.38
- Integrated all the latest changes from Fabrice Frances: Fixes some issues
  found by Goyo and Retroric regarding encoding of data tables by the compiler

1.39
- Fix to allow inline assembler directives to be inserted at the proper
  location in the functions

1.40 - 2026/04/19
- The compiler now reports a proper error when an expression is too complex
  and runs out of temporary registers (max 8), instead of silently emitting
  invalid assembly code (****** placeholders)

2.0
- Fixed a regression introduced by the 1.40 temporary-register change: all 32
  floating point temporaries were marked permanently busy, so ANY float
  expression failed with "expression too complex". Float temporaries are
  allocated on demand in the stack frame (the leading '*' in their name is the
  "not allocated yet" marker tested by local()), they must not be treated like
  the zero page integer temporaries.
- Added support for 0b/0B binary integer literals (C23 / common compiler
  extension), e.g. 0b1100101011111110 == 0xCAFE. Previously these produced
  a syntax error (found while investigating the oricCompilerBenchmark
  "0xcafe" sample failure).
- Fixed a -O3 wrong-code bug: passing a dereferenced computed pointer to a
  parameter of a different integer type (e.g. f(*(arr + i)) with f taking
  unsigned) passed the ADDRESS instead of the value. Two combined defects:
  the conversion no-op elision compared only operand/result names, ignoring
  the addressing mode (so "(tmp0),0" was confused with "tmp0"), and the
  temporary borrowed by a folded INDIR was released too early, letting the
  parent allocate the same slot and clobber the pointer while dereferencing
  it. Conversions now compare modes too, and the borrowed temporary stays
  reserved until the parent has allocated its own result.
- Fixed another -O3 temporary lifetime bug of the same family, exposed once
  struct code could finally link at -O3: when an address computation was
  consumed by a folded INDIR, a later unrelated node could allocate the same
  temporary and overwrite the address before the dereference (e.g. in
  "p->a == a && p->b == b && p->c == c" the pointer held in tmp0 was
  clobbered by the next subexpression). The borrow reservation now applies
  exactly when the fold consumed the last reference to its child; a shared
  child keeps sole ownership of its temporary (releasing it early was as
  harmful as not reserving it).
- Right shift of a signed value now performs an ARITHMETIC shift (the sign
  bit is replicated), matching every mainstream compiler: -16 >> 2 == -4.
  It used to emit the same logical shift as unsigned >>, giving 16380.
  Signed >> now emits the new ASRW macro family, unsigned >> keeps RSHW.
- Enabled stabline() to emit .csource directives when compiling with -g1 or
  higher. Each C source line now produces a .csource "filename" linenum
  annotation in the compiler output, allowing the assembler to map generated
  code back to original C source locations for debugging.
- Emit .ctype annotations under -g: the compiler now writes C variable and type
  information (names, and types including pointers, arrays and structs) into the
  output so the debugger can present typed variables. Types are collected during
  code generation and flushed with typedef-name resolution. Non-debug builds are
  unaffected.
- New #pragma optimize(push, n) / #pragma optimize(pop) / #pragma optimize(n)
  to change the optimization level (0-3) for the function definitions that
  follow, overriding the command line -On option. Typical use: wrap a function
  with optimize(push, 1) / optimize(pop) so its locals stay addressable on the
  stack frame for the debugger while the rest of the project builds at -O2, or
  selectively enable -O3 on functions known to survive it. The pragma is
  ignored (with graceful degradation) by older compiler versions.
- A .csource directive is now emitted right after each function label, tagging
  the entry code (ENTER prologue) with the function's definition line. It used
  to inherit the last marker of the PREVIOUS function, so debuggers mapped the
  entry address to that function's closing brace.

*/

#define TOOL_VERSION_MAJOR	2
#define TOOL_VERSION_MINOR	0
