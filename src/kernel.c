#include <stddef.h>
#include <stdint.h>
#include "libc/uart/uart.h"
#include "libc/misc.h"
#include "libc/debug/debug.h"
#include "interrupt.h"
#include "memory/alloc.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

void print_formatting_tests() {
	uart_info("Performing printf formatting tests:\r\n");
	uart_info("\tShould output \"hey\": %s\r\n", "hey");
	uart_info("\tShould output \"2a\": %x\r\n", 42);
	uart_info("\tShould output \"101010\": %b\r\n", 42);
	uart_info("\tShould output \"52\": %o\r\n", 42);
	uart_info("\tShould output \"-42\": %d\r\n", -42);
	uart_info("\tShould output 2 ** 32 + 1: %b\r\n", (((uint64_t) 1) << 32) + 1);
	uart_info("\tShould output \"100000001\": %x\r\n", (((uint64_t) 1) << 32) + 1);
	uart_info("Done testing printf formatting\r\n");
}

void debug_test(){
    assert(0);
}


void syscall_test(){
    asm volatile ("svc #0x1"::);
}

void malloc_test(){
    uart_debug("Entering malloc test\r\n");
    uint64_t * p = (uint64_t *) kmalloc(sizeof(uint64_t));
    *p = 42;
    char * array = (char *) kmalloc(GRANULE * sizeof(char));
    array[GRANULE - 1] = 42;
    ksbrk(-((int)(GRANULE * sizeof(char) + sizeof(uint64_t))));
    uart_debug("Done malloc test\r\n");

}
void kernel_main(uint64_t r0, uint64_t r1, uint64_t atags)
{
	// Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;
        uart_info("Beginning kernel initialization\r\n");
        init_alloc();
	uart_info("Performed kernel initialization\r\n");

	print_formatting_tests();

	//uint64_t variable = 0;
	//asm volatile("ADR %0, ." : "=r"(variable) : :);
	//uart_printf("address is : %x\r\n", variable);
	print_reg(MAIR_EL1);
        malloc_test();


	syscall_test();
	while (1){
		uart_putc(uart_getc());
	}
}
