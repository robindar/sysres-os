#include "../libc/uart/uart.h"
#include "../libc/debug/debug.h"

void proc0_main(){
    uart_printf("Init process running\r\n");
    abort();
}
