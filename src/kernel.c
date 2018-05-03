#include <stddef.h>
#include <stdint.h>
#include "libc/uart/uart.h"
#include "libc/misc.h"
#include "libc/debug/debug.h"
#include "interrupt.h"
#include "memory/alloc.h"
#include "test/test.h"
#include "proc/proc.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

void kernel_init(){
    uart_info("Beginning kernel initialization\r\n");
    init_alloc();
    init_proc();
    uart_info("Performed kernel initialization\r\n");
}

void kernel_main(uint64_t r0, uint64_t r1, uint64_t atags)
{
	(void) r0;
	(void) r1;
	(void) atags;
        kernel_init();
        free_test();
        abort();
        /* Start init process */
	/* exec_proc(1); */
}
