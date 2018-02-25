#include <stddef.h>
#include <stdint.h>
#include "libc/uart/uart.h"
#include "libc/misc.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

void print_formatting_tests() {
	uart_printf("\nPerforming printf formatting tests:\n");
	uart_printf("\tShould output \"hey\": %s\n", "hey");
	uart_printf("\tShould output \"2a\": %x\n", 42);
	uart_printf("\tShould output \"52\": %o\n", 42);
	uart_printf("\tShould output \"-42\": %d\n", -42);
	uart_printf("Done testing printf formatting\n");
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
	// Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

	uart_init();
	uart_printf("Performed kernel initialization\n");
	print_formatting_tests();

	while (1){
		uart_putc(uart_getc());
	}
}
