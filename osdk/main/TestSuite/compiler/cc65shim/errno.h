/* errno.h shim for the cc65 test/val import (single translation unit). */
#ifndef _ERRNO_H
#define _ERRNO_H

static int errno;

#define EDOM    1
#define ERANGE  2
#define EILSEQ  3
#define ENOMEM  4
#define EINVAL  5

#endif
