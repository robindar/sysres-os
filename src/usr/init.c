#include "../libk/debug.h"
#include "../libk/uart.h"

void proc0_main(){
    uint64_t x;
    uint64_t * p = (uint64_t *) 0x4300000;
    x = *p;
    *p = x;
    /* p = 0x7000; */
    /* x = *p; */
    uart_debug("x = 0x%x\r\n", x);
    uart_info("Init process running\r\n");
    uart_info("Halting...\r\n");
    /* Halt syscall : TODO : do a lovely interface */
    asm volatile("svc #100");
}
