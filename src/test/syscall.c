#include "test.h"
#include "../libk/debug.h"
#include "../usr/libc.h"
#include "../libk/uart.h"
#include "../libk/sys.h"
#include "../libk/errno.h"

#define TESTER_PID 5

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
    BEGIN_TEST();
    int ret = fork(0);
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        /* id_syscall_test(); */
        /* matrix_main(); */
        exit(0,0);
    }
    else{
        /* Parent */
        assert(ret == 4);
        uart_info("Parent process running\r\n");
        /* id_syscall_test(); */
        /* matrix_main(); */
        err_t exit_status;
        int pid = wait(&exit_status);
        assert(exit_status.data == 0);
        assert(exit_status.no == 0);
        assert(pid == ret);
        END_TEST();
        return;
    }
}

void fork_test2(){
    uart_verbose("Beginning fork test 2\r\n");
    assert(get_curr_pid() == TESTER_PID);
    int ret = fork(13);
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        exit(42,43);
    }
    else{
        /* Parent */
        assert(ret == 4);
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
void fork_test2bis(){
    BEGIN_TEST();
    assert(get_curr_pid() == TESTER_PID);
    int ret = fork(15);
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        exit(42,43);
    }
    else{
        /* Parent */
        assert(ret == 4);
        uart_info("Parent process running\r\n");
        err_t ret_data;
        int child_pid = wait(&ret_data);
        assert(child_pid     == ret);
        assert(ret_data.no   == 42);
        assert(ret_data.data == 43);
        END_TEST();
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
        (void) wait(NULL);
        assert(ret == 0);
    }
    END_TEST();
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
        (void) wait(NULL);
        assert(ret == 0);
        END_TEST();
    }
}

/* TODO: this fails */
void fork_test3(){
    BEGIN_TEST();
    int ret = fork(14);
    static int i = 0;
    uart_verbose("Return value of fork : %d\r\n");
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        uart_info("Child process running\r\n");
        uart_verbose("i: %d\r\n", i);
        i = 1;
        exit(0,0);
    }
    else{
        /* Parent */
        uart_info("Parent process running\r\n");
        /* if child executes before, assert(i == 0) fails */
        uart_verbose("i: %d\r\n");
        i = 1;
        (void) wait(NULL);
    }
    END_TEST();
    return;
}

void fork_test4(){
    uart_verbose("Bgeinning fork test 4");
    int n = 15;
    int ret;
    assert(get_curr_pid() == TESTER_PID);
    for(int i = 0; i < n; i++){
        ret = fork(0);
        assert(ret != -1);
        if(ret == 0) break;
    }
    if(ret == 0) exit(1,1);
    if(ret != 0){
        err_t r;
        int pid;
        for(int i = 0; i < n; i++){
            pid = wait(&r);
            uart_verbose("Heard from %d\r\n", pid);
            assert(r.data == 1);
            assert(r.no   == 1);
        }
    }
    uint64_t return_addr = (uint64_t)__builtin_return_address(0);
    uart_debug("Ret addr: %x\r\n", return_addr);
    uart_verbose("Done fork test 4\r\n");
}

/* fork_test4bis and sched_test1 fail but see doc/bugs.txt */
void fork_test4bis(){
    BEGIN_TEST();
    int n = 10;
    int ret;
    assert(get_curr_pid() == TESTER_PID);
    for(int i = 0; i < n; i++){
        ret = fork(0);
        assert(ret != -1);
        if(ret == 0) break;
    }
    if(ret == 0) {
        delay(1 << 20);
        exit(1,1);
    }
    if(ret != 0){
        err_t r;
        int pid;
        for(int i = 0; i < n; i++){
            pid = wait(&r);
            uart_verbose("Heard from %d\r\n", pid);
            assert(r.data == 1);
            assert(r.no   == 1);
        }
    }
    uart_debug("Ret addr: %x\r\n", (uint64_t)__builtin_return_address(0));
    /*uart_debug("FP: %x\r\n", (uint64_t)__builtin_frame_address(0)); */
    assert(print_children_from_pid(get_curr_pid()) == 0);
    END_TEST();
    return;
}

void sched_test1(){
    uart_printf("Beginning sched_test1\r\n");
    int ret;
    int n = 5;
    ret = fork(13);
    assert(ret != -1);
    int first_child = ret;
    if(ret != 0){
        for(int i = 1; i < n; i++){
            ret = fork(13);
            assert(ret != -1);
            if(ret == 0) break;
        }
    }
    if(ret != 0){
        for(int i = 0; i < n; i++){
            ret = fork(12);
            assert(ret != -1);
            if(ret == 0) break;
        }
    }
    if(ret != 0){
        err_t ret_data;
        int pid;
        for(int i = 0; i < 2*n; i++){
            pid = wait(&ret_data);
            uart_verbose("Heard from child: %d\r\n", pid);
            assert(ret_data.data == 1);
            assert(ret_data.no   == 1);
        }
    }
    if(ret == 0){
        delay(1 << 23);
        exit(1,1);
    }
    /* uint64_t return_addr = (uint64_t)__builtin_return_address(0); */
    /* uart_debug("Ret addr: %x\r\n", return_addr); */
    assert(print_children_from_pid(get_curr_pid()) == 0);
    uart_printf("Done sched_test1\r\n");
    return;
}

#define DUMP_STACK() asm volatile("mov %0, sp":"=r"(sp));\
    uart_debug("Dumping stack with sp: 0x%x\r\n", sp); \
    for(int i = 0; i < 5; i++){ \
        uart_debug("At 0x%x: 0x%x\r\n", sp + i * 8, AT(sp + i * 8));\
    }\

void test_copy_and_write(){
    uint64_t sp;
    BEGIN_TEST();
    DUMP_STACK();
    int ret = fork(15);
    DUMP_STACK();
    if(ret){
        uart_debug("Parent running\r\n");
        (void) wait(NULL);
    }
    else
    {
        uart_debug("Child running\r\n");
        DUMP_STACK();
        exit(0,0);
    }
    END_TEST();
}
