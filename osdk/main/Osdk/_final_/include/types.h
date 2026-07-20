#ifndef __TYPES_H__
#define __TYPES_H__

/*
 Fixed-width types plus the classic 8-bit peek/poke helpers. Portable
 6502 code (e.g. the ISS MOS6502 compiler benchmark) expects <types.h>
 to provide both.
*/

#include <stdint.h>

#define MEMPTR(address)      ((uint8_t*)(address))
#define peek(address)        (MEMPTR(address)[0])
#define poke(address,value)  (MEMPTR(address)[0]=((uint8_t)(value)))

#endif /* __TYPES_H__ */
