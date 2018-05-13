#include "test.h"
#include "../libk/debug.h"
#include "../usr/libc.h"
#include "../libk/uart.h"
#include "../libk/sys.h"

void id_syscall_test(){
    uart_verbose("Entring id syscall test\r\n");

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

/* Passed for both child and father (without scheduler) */
void fork_test1(){
    int ret = fork(0);
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        id_syscall_test();
        matrix_main();
        SYSCALL(100);
    }
    else{
        /* Parent */
        uart_info("Parent process running\r\n");
        id_syscall_test();
        matrix_main();
        SYSCALL(100);
    }
}

void fork_test2(){
    uart_verbose("Beginning fork test 2\r\n");
    int ret = fork(0);
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        exit(42,43);
    }
    else{
        /* Parent */
        uart_info("Parent process running\r\n");
        err_t ret_data;
        int child_pid = wait(&ret_data);
        assert(child_pid     == ret);
        assert(ret_data.no   == 42);
        assert(ret_data.data == 43);
        uart_verbose("Done fork test 2\r\n");
        return;
    }
}
