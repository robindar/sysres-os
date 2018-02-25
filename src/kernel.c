#include <stddef.h>
#include <stdint.h>
#include "libc/uart/uart.h"
#include "libc/misc.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
	// Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;

	uart_init();
	uart_printf("Running on CPU :%d\n %s %x %o %d", get_cpu_id(), "hey", 42, 42, -42);

	while (1){
		uart_putc(uart_getc());
	}
}
