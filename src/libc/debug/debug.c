#include "debug.h"
#include "../uart/uart.h"
#include <stdint.h>

__attribute__((__noreturn__))
void abort(){
    /*Prints the adress of the call site of abort*/
    uart_error("Kernel Panic\r\nAbort was called at : %x\r\n",
                ((uint64_t)__builtin_return_address(0)) - 4);
    asm volatile("b halt");
    /* So that GCC is sure this does not return */
    __builtin_unreachable ();
}
