#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../test/test.h"



void proc0_main(){
    /* uint64_t x; */
    /* uint64_t * p = (uint64_t *) 0x4300000; */
    /* x = *p; */
    /* *p = x; */
    /* uart_debug("x = 0x%x\r\n", x); */
    uart_info("Init process running\r\n");
    /* matrix_main(); */
    uart_info("Halting...\r\n");
    /* Halt syscall : TODO : do a lovely interface */
    asm volatile("svc #100");
}
