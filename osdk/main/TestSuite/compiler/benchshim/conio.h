/* conio.h shim for the ISS oricCompilerBenchmark samples.
   Mirrors playground/libmos6502vm/conio.h: _putc is a single store to a
   character port, so the I/O cost is comparable across toolchains. */
#ifndef __CONIO_H__
#define __CONIO_H__

#include "types.h"

#define LF '\x0a'
#define CR '\x0d'

void _putc(char c);
void _puts(const char* s);

#endif /* __CONIO_H__ */
