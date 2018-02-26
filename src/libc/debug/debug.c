#include "debug.h"
#include "../uart/uart.h"

__attribute__((__noreturn__))
void abort(){
    /*Prints the adress of the call site of abort*/
    uart_printf("Kernel Panic\nAbort was called at : %x\n",
                ((int)__builtin_return_address(0)) - 4);
    while(1){};
}

void assert(int b){
    if(b == 0){
        uart_printf("Assertion failed at : %x\n",
                    ((int)__builtin_return_address(0)) - 4);
        abort();
    }
}
