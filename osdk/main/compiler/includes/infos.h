
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

*/

#define TOOL_VERSION_MAJOR	1
#define TOOL_VERSION_MINOR	40
