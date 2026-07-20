/* types.h shim for the ISS oricCompilerBenchmark samples. */
#ifndef __TYPES_H__
#define __TYPES_H__

typedef signed char    int8_t;
typedef unsigned char  uint8_t;
typedef int            int16_t;
typedef unsigned int   uint16_t;
typedef long           int32_t;
typedef unsigned long  uint32_t;

/* MEMPTR/peek/poke: the benchmark's real-hardware variant (no VM array),
   so accesses go to actual Oric addresses. */
#define MEMPTR(address)       ((uint8_t*)(address))
#define peek(address)         (MEMPTR(address)[0])
#define poke(address,value)   (MEMPTR(address)[0]=((uint8_t)(value)))

#endif /* __TYPES_H__ */
