#ifndef __COMPAT_H__
#define __COMPAT_H__

/*
 Cross-compiler shims used by portable 6502 benchmark code. OSDK has no
 special divide intrinsic, so these are plain C operators.
*/

#define _div16u(dividend,divisor)      ((dividend)/(divisor))
#define _modr16u(dividend,divisor,rem) ((dividend)%(divisor))

#endif /* __COMPAT_H__ */
