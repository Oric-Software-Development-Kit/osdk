/* stdlib.h */

#ifndef _STDLIB_

#define _STDLIB_


#ifdef __OLD__

#include <old/stdlib.h>

#else


/* Include alloc.h, the memory allocation functions. */

#include <alloc.h>



/* The NULL pointer. */

#ifndef NULL
#define NULL ((void*)0x0000)
#endif


/* Exit the program. Return an exit code of retval. */

   /* retval is currently ignored on the Oric. The */
   /* operating system has no need for it.         */

extern __fastcall void exit(int retval);

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


/* Convert an integer i to a string */

extern char *itoa(int i);

/* Convert the initial portion of a string to an int: skips leading
   whitespace, accepts an optional sign, stops at the first non-digit. */

extern __fastcall int atoi(const char *s);

/* random generator */

#define RAND_MAX 32767

__fastcall int rand(void);
__fastcall int random(void);    /* rand and random are the same function */

int srandom(int seed); /* initialize the random generator */

#endif /* __OLD__ */

#endif /* _STDLIB_ */

/* end of file stdlib.h */
