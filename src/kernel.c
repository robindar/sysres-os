#include <stddef.h>
#include <stdint.h>
#include "libc/uart/uart.h"
#include "libc/misc.h"
#include "libc/debug/debug.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

void print_formatting_tests() {
	uart_printf("\r\nPerforming printf formatting tests:\r\n");
	uart_printf("\tShould output \"hey\": %s\r\n", "hey");
	uart_printf("\tShould output \"2a\": %x\r\n", 42);
	uart_printf("\tShould output \"52\": %o\r\n", 42);
	uart_printf("\tShould output \"-42\": %d\r\n", -42);
	uart_printf("Done testing printf formatting\r\n");
}

void debug_test(){
    assert(0);
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
	// Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

	uart_init();
	uart_printf("Performed kernel initialization\r\n");
	print_formatting_tests();

	while (1){
		uart_putc(uart_getc());
	}
}
