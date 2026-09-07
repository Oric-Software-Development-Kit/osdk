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
- Fixed #print failing with "Illegal pointer arithmetic" when mixing absolute values and relocatable addresses (e.g. #print DEFINE - *). 
  The pointer arithmetic check is now bypassed for #print, matching the existing behavior of #if.
- Fixed the last line of a source file being silently skipped when it lacks a trailing newline.
  The preprocessor now preserves non-empty lines terminated by EOF instead of discarding them.
- Numeric literals with trailing invalid characters now produce a syntax error instead of being silently accepted. Affects hex ($FF), binary (%01), and octal (&77) literals.
  For example, $FFg previously parsed as $FF with the trailing 'g' silently ignored.
- Fixed 0x hex prefix parsing reading from the wrong offset (skipped only the '0', not the 'x'), causing 0xFF to be parsed as value 0.
- 65816 instructions are now disabled by default. Values exceeding 16 bits (e.g. $BB80B from a typo) now produce an error instead of silently generating 65816 long addressing opcodes.
  Use '-w' (lowercase) to explicitly enable 65816 mode. The '-W' flag is kept for compatibility.
- Fixed division by zero in expression evaluator checking the wrong variable (always passed), causing undefined behavior instead of reporting an error.
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
- Added cheap local labels: @-prefixed labels (e.g. @loop, @done) are scoped to the nearest preceding standard label definition. 
  Each new standard label resets the cheap local scope, so multiple routines can reuse common names like @loop without collision.
- Added unnamed labels (-a flag): bare ':' defines a positional label, ':+' and ':-' reference the next/previous unnamed label, ':++'/':--' skip one, etc.
  The -a flag implies -M (MASM mode) since ':' is normally used as a statement separator.
- Added -E flag: exports global symbols as equates (name = $addr format, valid XA source).
  Optional -P flag filters exports by name prefix (e.g. -P _ for public API symbols).
  Optional -X flag excludes symbols found in a symbol file (e.g. -X symbols_Loader to strip imported symbols).
- Fixed #define macro expansion inside quoted strings: macros are no longer substituted inside
  "..." string literals in .byt directives. Previously, .byt "Use SET_TEXT" would silently
  expand SET_TEXT if it was #defined, corrupting string data with no error.
- Added str() preprocessor function for explicit macro-to-string conversion.
  str(NAME) expands to "value" where value is the macro's replacement text.
  Works with #define and -D command-line defines. If NAME is not defined, produces "NAME".
  Example: .byt str(VERSION) with -DVERSION=1.3.2 produces "1.3.2" in the output.
- The .align pseudo-opcode now works in absolute mode (previously only relocatable/o65).
  Accepts any power-of-2 value (e.g. .align 8, .align 16, .align 256 for page alignment).
  Optional fill byte parameter: .align 256, $EA (default fill is 0).
- Preprocessor directives can now have leading whitespace, matching standard C behavior.
  Previously, the '#' had to be the very first character on the line or the directive was ignored.
- Emitting code or data in the .zero segment now produces an error. The .zero segment is only
  for reserving space with .dsb (no fill byte). Use .text with *= for executable zero-page code.
- Zero page address overflow in indirect/direct page modes now produces a specific error message
  instead of the generic "Overflow", making it clear the address exceeds the $00-$FF range.
- Improved error message formatting: consistent "file(line):addr: message" layout.

2.4.0 - 2026/07/27
- The twelve section boundary labels are excluded from -E symbol export. They describe the
  layout of one assembly unit, so exporting them injected one unit's boundaries into every
  unit that included the generated header, where they silently collided with that unit's
  own set. They remain in the -l and -S symbol files, which are per-unit anyway.
- Added __bss_clear_start / __bss_clear_end / __bss_clear_size, covering the auto-chained
  .bss run only (the reservations made before any *= pinned the .bss PC). This is the range
  a C runtime may safely zero at startup; __bss_end is start+total-length and therefore
  spans *=-pinned blocks, so it must never be used for that.
- The auto-chained .bss base is rounded up to a page boundary. It costs no file bytes, since
  .bss is reserved and never emitted, and it lets a startup clear walk whole pages while
  giving whatever lands first in .bss an indexed access that cannot cross a page.
- Unbalanced #if / #ifdef / #endif is now a fatal error pointing at the exact directive.
  A missing #endif used to silently swallow everything after it, which is an efficient way
  to lose an entire library without noticing.
- A segment whose contents would run past the end of the 64K address space is now an error
  instead of quietly wrapping around. A zero-length segment landing at $10000 stays silent,
  since a unit whose code ends at the top of memory places nothing there.
- Fixed .align segment base address warnings firing in absolute mode where they are irrelevant.
  The warnings (and o65 header alignment flags) now only apply in relocatable mode (-R).
  Also changed alignment tracking from global to per-segment, so a .align in one segment
  no longer triggers spurious warnings about unrelated segments.
- Fixed comparison operators (<, >, <=, >=, ==, !=) between labels causing "Illegal pointer
  arithmetic" errors. Comparing any two addresses now works regardless of segment.
- Improved .assert/.asserteq error reporting: the assertion message is now included on the
  same line as the file/line/address information instead of being printed on a separate line.
- Added debug symbol output for the VS Code / GDB debugger (new -S <file> option). Writes an
  extended "#SYM V2" symbol file listing each symbol with its source file and line, plus a
  #FILES index, a #LINES address->(file,line) map, and a #TYPES section. Symbol source
  locations are captured at definition time. Emitted only when -S is given; without it the
  output is byte-for-byte unchanged.
- Added .csource directive support for C source line mapping. The C compiler
  emits .csource "filename" linenum directives which the assembler intercepts
  during line reading (alongside #file/#line), recording T_CSOURCE tokens in
  the intermediate buffer. During pass 2, T_CSOURCE sets the current file/line
  for the line table, and subsequent T_LINE events are suppressed while a C
  source mapping is active (preventing intermediate .s file line numbers from
  overwriting the C source coordinates). T_FILE events reset to normal
  assembly tracking.
- Added C 'enum' support in the preprocessor. A 'enum { ... }' or
  'typedef enum { ... } Tag;' declaration (which XA previously rejected as a
  syntax error) now has each enumerator registered as a #define: explicit
  '= <int>' sets the running value (hex/octal literals are normalised to
  decimal so the assembler can read them), an omitted value is previous+1
  (0 for the first), and a non-integer '= <expr>' is passed through verbatim.
  The tag/typedef name is discarded. The whole declaration is skipped inside a
  not-taken #if branch, and preprocessor conditionals INSIDE the body
  (#ifdef/#ifndef/#else/#endif) are honoured per line, so conditional members
  work (an excluded member is dropped and does not advance the running value).
  Macro-valued members are expanded. (Backslash line-continuation and #include
  inside the body are not handled.) This lets a header shared by the C compiler
  and the assembler use a single real enum instead of a parallel #define list.
- Fixed '.ctype' debug directives being macro-expanded. The C compiler emits
  .ctype records (for the -S symbol file) with literal enumerator/field/type
  names; these are now passed through verbatim instead of going through macro
  replacement. Previously, when the same enum lived in a header shared by C and
  assembler, the assembler side registered each enumerator as a #define (see
  above), so an enumerator name appearing in a .ctype line got rewritten to its
  value -- e.g. "enum KeyboardLayout KEYBOARD_QWERTY=0 ..." became
  "enum KeyboardLayout 0=0 ...", corrupting the debugger's value->name map.
- The #SYM V2 source location of a label is now its DEFINITION line. It used to
  be the name's first textual occurrence: for a forward-referenced label (a
  'jmp _Label' before the label) the recorded location was that reference,
  sending debugger navigation to a call site instead of the definition. The
  location is re-stamped when the label actually receives its value (position
  label or '=' assignment); symbols never defined in the unit (imports) keep
  the first-occurrence location as the best available. Pass 2 has no
  preprocessor context and leaves the pass-1 stamp untouched.
- Automatic segment chaining in absolute mode (Devpac-style). The .data segment
  now starts right after the end of .text, and .bss right after .data, computed
  from the measured segment sizes - so sources no longer need the manual "capture
  an end-of-text label, then force *= that label at the top of .bss" pattern to
  place uninitialised data / the stack above the code. Once every user file is
  assembled, each following segment's base is set to the previous segment's end
  and its labels are shifted to match; pass 2 re-evaluates operands and emits the
  new addresses. Only labels sitting at the NATURAL segment PC move: as soon as a
  source line pins the PC with a "*=" directive, that label and every later one in
  the segment keep their explicit address (screen / overlay / hardware placements
  are left untouched). .text and .zero bases are never moved. Twelve boundary
  labels are published for use in emitted code: __text_start/__text_end/__text_size
  and the same for __data_/__bss_/__zero_. NOTE: these boundary labels, like the
  chained .data/.bss addresses, are only final AFTER pass 1, so they must not be
  used in pass-1 constructs (#if / #print / #error / .dsb count); a #print of "*"
  or a label in .text is unaffected (the text base never moves).

2.4.1
- Fixed auto-chaining emitting a stale address for any reference to a .data/.bss label
  that appeared AFTER the block defining it. Such a label was already in the symbol table
  when t_conv tokenised the reference, so its provisional value - SectionBssBase ($4000)
  plus the offset within the segment - was substituted into the token stream and from
  there into the machine code. Auto-chaining then relocated the symbol TABLE only, so the
  binary and the exported symbols disagreed with no error reported: a project could
  execute "inc $401C" against a variable the symbol file placed at $121C. References
  appearing BEFORE the defining block were unaffected, which made the failure look
  arbitrary. The note above is precise that pass 2 "re-evaluates operands", but that only
  holds for operands pass 1 could not resolve; a resolvable line is assembled in pass 1
  and its bytes are replayed verbatim. Such a reference is now kept symbolic so pass 2
  resolves it against the final address. Constructs that legitimately need a value during
  pass 1 (#if, "*=", .dsb, .assert, "=") are untouched and still resolve as before.
- Output in a .bss segment is now an error instead of being silently dropped. .bss
  reserves without emitting, so a .byt / .word / .asc / instruction there produces nothing
  and is then zeroed by a C runtime that clears the section. The usual cause is a missing
  ".text" at the top of a file assembled after one that ended in .bss. Mirrors the
  existing .zero rule; .dsb and .align remain allowed.
- #print now reports the FINAL address of an auto-chained .data/.bss label instead of the
  provisional one. Being a pass-1 construct, #print used to evaluate before segment
  chaining had moved .data and .bss into place, so it printed the pass-1 base ($4000 for
  .bss) plus the label's offset within the segment: a project asking "#print Main RAM used
  up to = _EndBSS" was told $64F4 while the symbol file and the running program agreed on
  $83F4, and the number did not even change between builds of different sizes. The value
  was then used to compute remaining memory, which made the report actively misleading.
  Since #print emits no bytes, nothing downstream depends on when it is answered: a line
  whose expression resolves a label auto-chaining may still move is now queued and printed
  once the segments have been relocated, immediately before pass 2. Everything else prints
  where it always did, so a #print of "*", of a .text label or of a pinned address keeps
  its position in the log next to the #echo lines around it. What is queued is the TOKEN
  stream produced at the directive, not the source text - re-tokenising after pass 1 would
  resolve the names outside the block and cheap-local scope the directive stood in - and
  the PC of the directive is captured with it, so a '*' in a mixed expression still means
  the address the #print stood at. Nothing is deferred in relocatable mode (-R), where no
  chaining happens. #if / #error / .dsb are deliberately untouched: they gate what pass 1
  assembles and therefore cannot be deferred, so a guard such as "#if _EndBSS > $9900"
  still compares the provisional value.
- __bss_end / __bss_size now describe the auto-chained .bss run only, matching the
  __bss_clear_* pair added in 2.4.0, instead of start+total-length. A "* = $XXXX" block
  inside .bss is a deliberate hand placement (screen, overlay, hardware) that opts out of
  chaining, so its bytes belong to no run and must not push the end of one: with a pack
  pinned at $C000, __bss_end landed in unrelated RAM and every size or free-memory figure
  derived from it read garbage. The total reservation is still counted for the
  relocatable-mode (-R) object header, which describes the whole segment.
- The "auto-chain: .bss runs past the top of memory" check no longer counts *=-pinned
  bytes either. Only the chained run is placed at the computed base, so including the
  pinned blocks in the sum reported an overflow for a layout that fits.

*/


#define TOOL_VERSION_MAJOR	2
#define TOOL_VERSION_MINOR	4
#define TOOL_VERSION_PATCH	1

#define _TOOL_XSTR(s)	_TOOL_STR(s)
#define _TOOL_STR(s)	#s
#define TOOL_VERSION_STRING	_TOOL_XSTR(TOOL_VERSION_MAJOR) "." _TOOL_XSTR(TOOL_VERSION_MINOR) "." _TOOL_XSTR(TOOL_VERSION_PATCH)
