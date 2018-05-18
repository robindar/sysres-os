#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../libk/sys.h"
#include "../test/test.h"



void main_init(){
    uart_info("Init process running\r\n");
    chan_test1();
    uart_info("Halting...\r\n");
    /* Halt syscall : TODO : do a lovely interface */
    SYSCALL(100);
}
