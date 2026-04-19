/*

The 6502 Linker, for the lcc or similar, that produce .s files
to be processed later by a cross assembler

List of modifications:

Change history for the Linker

0.00:
- Originaly created by Vagelis Blathras

0.56 
- Handling of lines that have more than 180 characters

0.57
- Added '-B' option to suppress inclusion of HEADER and TAIL

0.58 - January 2004
- Added filtering of all '#' directives
- Added an icon to the executable file to make it more 'OSDK' integrated :)
- Added '-F' option to enable #file directive (requires modified XA assembler)
- Modified the handling of comments to avoid crashes on C and C++ comments

0.59 - June the 4th 2006
- Corrected a bug that made it impossible to 'link' only one source file

0.63 - 2009-02-14
Fixed a number of issues in the linker:
- (WIP) The old linked filtered out comments, need to implement this feature as well
- removed some test code
- fixed the loading of symbols from the library index  file
- Fixed a problem of text file parsings. Mixed unix/dos carriage return would result in very long lines (containing many lines), leading to some crashes later on.
- Also fixed a problem in reporting the parsed files.

0.64 - 2016/01/17
- Fixed the age old problem if includes from assembler sources leading to Unresolved External errors

0.65 - 2017/03/18
- Fixed some issues in the token pattern matching used to detect labels resulting in #includes containing relative paths to be incorrectly parsed

0.66 - 2019/04/06
- The new macro file generate lines that contain multiple instructions, the linker stopped at the first encountered instruction, this new version correctly parses that

0.67 - 2020/12/18
- Error messages now indicate that they came from the Linker, because a generic "Error, can't load xxxx" was not very informative

0.68 - 2023/01/18
- The 'Unresolved external: <name>' now also specify the filename and line number of the first place where the symbol was requested, as well as the total number of references.

1.0 - 2023/09/24
- Added a "-i" option to provide alternative include file paths

1.1 - 2023/09/24
- Added support for a "#pragma osdk replace_characters" directive used to perform localization of texts (experimental)

1.2 - 2026/02/22
- Added support for a "#pragma osdk replace_characters_if LANGUAGE_TAG" directive and a "-r LANGUAGE_TAG" command line option
  to perform conditional character replacement: only the pragma whose tag matches the -r argument is applied.
  This allows multiple language pragmas to coexist in a single file without the last one overriding all others.

1.3 - 2026/03/01
- Fixed a bug where a comment on a line containing a string requiring localization would prevent the character replacement from being applied

1.4 - 2026/04/26
- Added "-S" option to import symbols from an XA symbol file (-l output).
  Imported symbols are written as equates in the output file and marked as defined,
  preventing library files that define those symbols from being pulled in.
  If the module source redefines an imported symbol, the module's definition wins.
- Added "-t symbolname" option to inject a .text origin from an imported symbol.
  When used with -S, Link65 emits ".text / * = symbolname" at the top of the output,
  setting the assembly origin for the module without needing -bt or OSDKADDR.
- Added "#pragma osdk import" directive to force-import library symbols without requiring dead code references
  Symbols listed after the pragma (space, tab, or comma separated) are treated as label references, causing Link65 to pull in the library files that define them.
  Example: #pragma osdk import _memset, _memcpy, mul16i, mul16u
- Added an error message for unrecognized "#pragma osdk" directives (previously silently ignored)
- Fixed false "Unresolved external" errors caused by C preprocessor macros in assembly files:
  - #define bodies (single-line with colon-separated instructions, and multi-line with backslash
    continuations) are no longer parsed as real code — only the macro name is registered
  - Fixed #H/#L high/low byte prefix detection matching labels starting with H or L
    (e.g. #LastCrane was incorrectly treated as #L prefix + label reference)
- Added "-g" option to filter -S symbol imports using an XA equates file (from XA -E output).
  Only symbols in the filter file are imported, preventing local kernel labels from causing
  name collisions in modules.
- Fixed "#pragma osdk" directives reporting the wrong filename after a #include (the recursive ParseFile call overwrote the current file context)
- Added support for XA extended label syntax in the label tokenizer:
  - & (block escape prefix) recognized as token delimiter for correct label name extraction
  - : unnamed label definitions (bare colon at line start) are skipped to prevent misparse

*/


#define TOOL_VERSION_MAJOR	1
#define TOOL_VERSION_MINOR	4
