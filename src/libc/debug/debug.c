#include "debug.h"
#include "../uart/uart.h"
#include <stdint.h>

__attribute__((__noreturn__))
void abort(){
    /*Prints the adress of the call site of abort*/
    uart_printf("Kernel Panic\r\nAbort was called at : %x\r\n",
                ((uint64_t)__builtin_return_address(0)) - 4);
    while(1){};
}

void assert(int b){
    if(b == 0){
        uart_printf("Assertion failed at : %x\r\n",
                    ((uint64_t)__builtin_return_address(0)) - 4);
        abort();
    }
}
