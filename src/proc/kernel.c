#include <stddef.h>
#include <stdint.h>
#include "proc.h"
#include "../libk/errno.h"
#include "../libk/filesystem.h"
#include "../libk/uart.h"
#include "../memory/alloc.h"
#include "../test/test.h"
#include "../interrupt/timer.h"

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

void kernel_init() {
    uart_info("Beginning kernel initialization\r\n");
    init_alloc();
    init_proc();
    init_filesystem();
    init_timer_irq();
    uart_info("Performed kernel initialization\r\n");
}

void kernel_main(uint64_t r0, uint64_t r1, uint64_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;
    kernel_init();
    /* Start init process */
    kernel_timer_test();
}
