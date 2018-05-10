#include "test.h"
#include "../libk/debug.h"
#include "../usr/libc.h"
#include "../libk/uart.h"

void id_syscall_test(){
    uart_verbose("Enetring id syscall test\r\n");

    uart_verbose("Simple var test\r\n");
    volatile uint64_t x = 42;
    SYSCALL(101);
    assert(x == 42);
    uart_verbose("Simple var test done\r\n");

    uart_verbose("Array test\r\n");
    volatile int array[32];
    for(int i = 0; i < 32; i++) array[i] = i;
    for(int i = 1; i < 32; i++) array[i] += array[i - 1];
    for(int i = 0; i < 32; i++)
        assert(array[i] == i * (i + 1) / 2);
    /* uart_debug("&array : 0x%x\r\n",&array); */
    SYSCALL(101);
    /* uart_debug("&array : 0x%x\r\n",&array); */
    for(int i = 0; i < 32; i++)
        assert(array[i] == i * (i + 1) / 2);
    uart_verbose("Array test done\r\n");
    uart_verbose("Leaving id syscall test\r\n");
}
