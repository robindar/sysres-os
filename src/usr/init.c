#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../libk/sys.h"
#include "../test/test.h"



void main_init(){
    uart_info("Init process running\r\n");
    proc_timer_test();
    uart_info("Halting...\r\n");
    /* Halt syscall : TODO : do a lovely interface */
    SYSCALL(100);
}
