#include "alloc.h"

void * ksbrk(unsigned int increment) {
	uint64_t end_addr = 0;
	asm volatile ("ldr %0, =__end" : "=r"(end_addr) : :);
	static uint64_t end_offset = 0;
	uint64_t res = end_offset + end_addr;
	end_offset += (uint64_t) increment;
	return (void *) res;
}
