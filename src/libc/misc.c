#include "misc.h"

// Loop <delay> times in a way that the compiler won't optimize away
void delay(uint32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
			: "=r"(count): [count]"0"(count) : "cc");
}

uint32_t get_cpu_id(){
	/*
	uint32_t res;
	asm("MRC p15,0,%0,c0,c0,5 ;":"=r"(res)::);
	return (res & 0xFF);
	*/
	return 0;
}
