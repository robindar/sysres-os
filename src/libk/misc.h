#include <stddef.h>
#include <stdint.h>

#ifndef MISC_H
#define MISC_H

#define one_u64 ((uint64_t) 1)
#define AT(addr) (* (uint64_t *) (addr))
#define MASK(f,t) ((f < t) ? _MASK_(t,f) : _MASK_(f,t))
#define _MASK_(f,t) ( f == 63 ? 0 : (one_u64 << (f+1)) ) - (one_u64 << t)

void delay(uint32_t count);
uint32_t get_cpu_id();
uint64_t get_current_address();

#endif
