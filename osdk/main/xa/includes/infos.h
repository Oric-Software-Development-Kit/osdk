/*

Cross-Assembler 65xx

Change history for XA

2.0.7c
- The include statements can now contain '\' or '/' as directory separator, and both are accepted.

2.0.7d
- Now understands the #undef preprocessor directive.

2.1.1e
- Started supporting the 'o65' file format.

2.1.2a
- Linker, relocator and file utility for 'o65' format included, some bugs fixed.

2.1.3
- A new feature of the fileformat: aligned segments. Segments can be aligned to 2, 4, or 256 byte address boundary.
- New pseudo-opcode ".align" aligns the address in the current segment to the value given.
- Pseudo-opcodes ".data", ".bss", ".zero" and ".text" to switch segments can now be used in absolute mode.

2.1.4
- Preprocessor understands continuation lines, more command line options available.

2.1.4e
- Some bugfixes concerning o65 fileformat, as well as a fileformat change.

2.1.4f (21apr1998)
- Cross-compilable for DOS with make dos.

2.1.4g (25nov1998)
- Fix "!" addressing syntax, add reference list for labels.

2.1.4h (25nov1998)
- Preprocessor fix, improve file65.

2.1.5 - January 2004
- Added a #file directive.
- Corrected some error messages.

2.1.6 - 2006/06/04
- Fixed the #file directive that was actually giving valid file names only for errors that happened during pass 1.

2.1.7
- Removed the '-x' switch. Use -o, -e and -l instead.

2.2.1
- Fixed a problem of data corruption happening when a line parsed during pass1 generated more than 127 bytes of tokens for pass2 to decode.

2.2.2 - 2011/01/15
- C, C++ and asm comments are now handled correctly and ignored if in a quoted string.

2.2.3 - 2011/01/18
- Fixed a crash happening when no filename was provided for the symbols or error files.

2.2.4 - 2017/03/18
- Extended the values for a number of hardcoded defines (labels, blocks, number of open files, etc...).

2.2.5 - 2017/04/13
- Added a new '-cc' command line parameter to allow compatibility with the CC65 toolchain.

2.2.6 - 2023/01/14
- Returns an error code value 1 if a source file is missing.

2.2.7 - 2025/04/19
- Extended the "#print" command to make the formatting of size output nicer by using the '=' symbol as a separator between the expression and some arbitrary string on the left hand side.

2.3.0 - 2026/02/28
- Added support for the defined() operator in preprocessor expressions.
- Added support for the #elif preprocessor directive.

2.3.1 - 2026/04/12
- Fixed the defined() operator not working in #if directives (only worked in #elif)
- Added #error preprocessor directive: stops assembly with a custom error message.
  Correctly skipped inside false #if/#ifdef branches. Supports macro expansion in message text.
- Fixed #print failing with "Illegal pointer arithmetic" when mixing absolute values and
  relocatable addresses (e.g. #print DEFINE - *). The pointer arithmetic check is now
  bypassed for #print, matching the existing behavior of #if.
- Fixed the last line of a source file being silently skipped when it lacks a trailing newline.
  The preprocessor now preserves non-empty lines terminated by EOF instead of discarding them.
- Numeric literals with trailing invalid characters now produce a syntax error instead of being
  silently accepted. Affects hex ($FF), binary (%01), and octal (&77) literals.
  For example, $FFg previously parsed as $FF with the trailing 'g' silently ignored.
- Fixed 0x hex prefix parsing reading from the wrong offset (skipped only the '0', not the 'x'),
  causing 0xFF to be parsed as value 0.
- 65816 instructions are now disabled by default. Values exceeding 16 bits (e.g. $BB80B from a
  typo) now produce an error instead of silently generating 65816 long addressing opcodes.
  Use '-w' (lowercase) to explicitly enable 65816 mode. The '-W' flag is kept for compatibility.
- Fixed division by zero in expression evaluator checking the wrong variable (always passed),
  causing undefined behavior instead of reporting an error.
- Added .assert pseudo-opcode: .assert expression, "message"
  Stops assembly with a custom message if expression evaluates to zero (false).
  Example: .assert *<$fffa, "code hit vectors!"
- Added .asserteq pseudo-opcode: .asserteq value, expected, "message"
  Stops assembly if value != expected, reporting both values in decimal and hex.
  Example: .asserteq EndTable-StartTable, 512, "table size wrong"
- Added .bin pseudo-opcode: .bin offset, length, "filename"
  Includes raw binary file data directly in the output without needing bin2txt conversion.
  Offset specifies the starting byte position, length the number of bytes (0 = rest of file).
  Validates file existence, bounds, and checks for 16-bit address overflow.

*/


#define TOOL_VERSION_MAJOR	2
#define TOOL_VERSION_MINOR	3
#define TOOL_VERSION_PATCH	0

#define _TOOL_XSTR(s)	_TOOL_STR(s)
#define _TOOL_STR(s)	#s
#define TOOL_VERSION_STRING	_TOOL_XSTR(TOOL_VERSION_MAJOR) "." _TOOL_XSTR(TOOL_VERSION_MINOR) "." _TOOL_XSTR(TOOL_VERSION_PATCH)
