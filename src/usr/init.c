#include "../libc/uart/uart.h"
#include "../libc/debug/debug.h"




void proc0_main(){
    uart_info("Init process running\r\n");
    uart_info("Halting...\r\n");
    abort();
}
