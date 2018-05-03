#include "../libc/uart/uart.h"
#include "../libc/debug/debug.h"




void proc0_main(){
    uint64_t x;
    uint64_t * p = 0x4300000;
    x = *p;
    *p = x;
    /* p = 0x7000; */
    /* x = *p; */
    uart_debug("x = 0x%x\r\n", x);
    uart_info("Init process running\r\n");
    uart_info("Halting...\r\n");
    abort();
}
