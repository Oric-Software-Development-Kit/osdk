#ifndef __STDINT_H__
#define __STDINT_H__

/*
 Fixed-width integer types for the OSDK C compiler.

 char/short are 8/16-bit, int is 16-bit and long is 32-bit, so these map
 cleanly onto the native types. (long became a true 32-bit type in OSDK
 1.24; before that int32_t/uint32_t were silently only 16 bits.)
*/

typedef signed char    int8_t;
typedef unsigned char  uint8_t;
typedef int            int16_t;
typedef unsigned int   uint16_t;
typedef long           int32_t;
typedef unsigned long  uint32_t;

#define INT8_MIN    (-128)
#define INT8_MAX    127
#define UINT8_MAX   255
#define INT16_MIN   (-32767-1)
#define INT16_MAX   32767
#define UINT16_MAX  0xffffu
#define INT32_MIN   (-2147483647L-1)
#define INT32_MAX   2147483647L
#define UINT32_MAX  0xffffffffuL

#endif /* __STDINT_H__ */
