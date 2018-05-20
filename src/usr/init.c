#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../libk/sys.h"
#include "../test/test.h"

void listen_shutdown(){
    int pid;
    int code;
    while(1){
        pid = receive(&code, sizeof(int));
        switch(code){
        case 1:
            uart_info("System halting at the request of process %d\r\n", pid);
            halt();
        default:
            /* Unknown signal */
            (void) acknowledge(-1, NULL, 0);
        }
    }
}


void main_init(){
    uart_info("Init process running\r\n");
    shutdown_test();
    listen_shutdown();
}

