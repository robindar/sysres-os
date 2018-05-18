#include "test.h"
#include "../libk/debug.h"
#include "../usr/libc.h"
#include "../libk/uart.h"
#include "../libk/sys.h"
#include "../libk/errno.h"

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

typedef struct {
    uint64_t data1;
    uint64_t data2;
} recv_t;

void chan_test1(){
    uart_verbose("Beginning chan test 1\r\n");
    int ret = fork(0);
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        recv_t sd;
        char ack[256];
        sd.data1 = 42;
        sd.data2 = (uint64_t) -1;
        int status = send(1, &sd, sizeof(recv_t), ack, 256, true);
        assert(status == 54);
        uart_verbose("Ack : %s\r\n", ack);
        assert(strcmp(ack, "Hello child") == 0);
        uart_debug("Test passed successfully\r\n");
        exit(0, 0);
    }
    else{
        /* Parent */
        uart_info("Parent process running\r\n");
        recv_t recv;
        char * ack = "Hello child";
        int pid = receive(&recv, sizeof(recv_t));
        assert(pid == 2);
        uart_debug("Parent heard from his or her child %d\r\n", pid);
        assert(recv.data1 == 42);
        assert(recv.data2 == (uint64_t)-1);
        int ret = acknowledge(54, ack, strsize(ack));
        assert(ret == 0);
        exit(0, 0);
    }
}

void chan_test2(){
    uart_verbose("Beginning chan test 2\r\n");
    uart_verbose("Sizeof enum %d\r\n", sizeof(errno_t));
    int ret = fork(0);
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        char * ack = "Yo parent";
        int status = send(1, 0, sizeof(recv_t), ack, 256, true);
        assert(status == 54);
        assert(strcmp(ack, "Yo parent") == 0);
        uart_verbose("Ack : %s\r\n", ack);
        uart_debug("Test passed successfully\r\n");
        exit(0, 0);
    }
    else{
        /* Parent */
        uart_info("Parent process running\r\n");
        recv_t recv;
        recv.data1 = 5;
        recv.data2 = 6;
        char * ack = "Hello child";
        int pid = receive(&recv, sizeof(recv_t));
        assert(pid == 2);
        uart_debug("Parent heard from his or her child %d\r\n", pid);
        uart_verbose("recv.data1: %x\r\n", recv.data1);
        assert(recv.data1 == 5);
        assert(recv.data2 == 6);
        int ret = acknowledge(54, 0, strsize(ack));
        assert(ret == 0);
        exit(0, 0);
    }
}
