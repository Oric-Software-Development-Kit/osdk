/* limits.h shim for the cc65 test/val import - documents the REAL limits
 * of the OSDK compiler, including its two deliberate deviations:
 * short is 8 bits and long is 16 bits. */
#ifndef _LIMITS_H
#define _LIMITS_H

#define CHAR_BIT    8
#define SCHAR_MIN   (-128)
#define SCHAR_MAX   127
#define UCHAR_MAX   255
#define CHAR_MIN    (-128)
#define CHAR_MAX    127

/* NON STANDARD: short is one byte in the OSDK compiler */
#define SHRT_MIN    (-128)
#define SHRT_MAX    127
#define USHRT_MAX   255

#define INT_MIN     (-32767-1)
#define INT_MAX     32767
#define UINT_MAX    65535u

/* NON STANDARD: long is 16 bits in the OSDK compiler */
#define LONG_MIN    (-32767-1)
#define LONG_MAX    32767
#define ULONG_MAX   65535u

#endif
