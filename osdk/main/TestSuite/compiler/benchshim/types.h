/* types.h shim for the ISS oricCompilerBenchmark samples.
   NOTE: OSDK's long is 16-bit, so int32_t/uint32_t are NOT 32 bits wide -
   samples relying on them (e.g. pi) build and run but compute wrong
   values, exactly as in the published benchmark results. */
#ifndef __TYPES_H__
#define __TYPES_H__

typedef signed char    int8_t;
typedef unsigned char  uint8_t;
typedef int            int16_t;
typedef unsigned int   uint16_t;
typedef long           int32_t;   /* 16-bit! see note above */
typedef unsigned long  uint32_t;  /* 16-bit! see note above */

#endif /* __TYPES_H__ */
