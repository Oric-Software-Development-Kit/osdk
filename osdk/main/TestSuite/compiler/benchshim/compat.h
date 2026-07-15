/* compat.h shim for the ISS oricCompilerBenchmark samples.
   Mirrors the non-kickc branch of playground/libmos6502vm/compat.h: the
   division helpers are plain C operators (OSDK has no special div intrinsic). */
#ifndef __COMPAT_H__
#define __COMPAT_H__

#define _modr16u(dividend,divisor,rem) ((dividend)%(divisor))
#define _div16u(dividend,divisor)      ((dividend)/(divisor))

#endif /* __COMPAT_H__ */
