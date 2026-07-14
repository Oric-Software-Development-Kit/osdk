/* stdint.h shim for the cc65 test/val import. Only 8 and 16 bit types:
 * the OSDK compiler has no 32 bit integer, so int32_t is deliberately
 * absent - tests requiring it fail to compile instead of running wrong. */
#ifndef _STDINT_H
#define _STDINT_H

typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef int             int16_t;
typedef unsigned int    uint16_t;

typedef int8_t          int_least8_t;
typedef uint8_t         uint_least8_t;
typedef int16_t         int_least16_t;
typedef uint16_t        uint_least16_t;
typedef int8_t          int_fast8_t;
typedef uint8_t         uint_fast8_t;
typedef int16_t         int_fast16_t;
typedef uint16_t        uint_fast16_t;

typedef int             intptr_t;
typedef unsigned int    uintptr_t;
typedef int             intmax_t;
typedef unsigned int    uintmax_t;

#define INT8_MIN    (-128)
#define INT8_MAX    127
#define UINT8_MAX   255
#define INT16_MIN   (-32767-1)
#define INT16_MAX   32767
#define UINT16_MAX  65535u
#define INTPTR_MIN  INT16_MIN
#define INTPTR_MAX  INT16_MAX
#define UINTPTR_MAX UINT16_MAX

#endif
