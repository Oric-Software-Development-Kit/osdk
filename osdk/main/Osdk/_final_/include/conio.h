#ifndef __CONIO_H__
#define __CONIO_H__

/*
 For contiki
*/

#define kbhit key
#define cgetc get

/*
 conio-style console output used by portable 6502 test/benchmark code
 (e.g. the ISS MOS6502 compiler benchmark). _puts does NOT append the
 trailing newline that the standard puts() adds. Implemented in lib/conio.s.
*/
void _putc(char c);
__fastcall void _puts(const char* s);	/* string pointer in A:X (see lib/conio.s) */

/* line-ending constants used by portable console code */
#define LF '\x0a'
#define CR '\x0d'

#endif /* __CONIO_H__ */
