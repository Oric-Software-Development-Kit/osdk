/* stddef.h shim for the cc65 test/val import. */
#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef NULL
#define NULL 0
#endif

typedef unsigned int size_t;
typedef int ptrdiff_t;

#define offsetof(type, member) ((size_t)&(((type*)0)->member))

#endif
