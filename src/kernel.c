#include <stddef.h>
#include <stdint.h>
#include "libc/uart/uart.h"
#include "libc/misc.h"
#include "libc/debug/debug.h"
#include "interrupt.h"
#include "memory/alloc.h"
#include "test/test.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

void kernel_init(){
    uart_info("Beginning kernel initialization\r\n");
    init_alloc();
    uart_info("Performed kernel initialization\r\n");
}

void kernel_main(uint64_t r0, uint64_t r1, uint64_t atags)
{
	// Declare as unused
	(void) r0;
	(void) r1;
	(void) atags;
        kernel_init();
	/* print_formatting_tests(); */
        malloc_test();
        matrix_main();

        debug_test();
	syscall_test();
	while (1){
		uart_putc(uart_getc());
	}
}
