#include "sys.h"

int fork(int priority){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "SVC #0;"\
        "mov %0, x0;"
        : "=r"(ret)
        : "r"((uint64_t)priority)
        : "x0");
    return (int) ret;
}

__attribute__((__noreturn__))
void exit(errno_t no, errdata_t data){
    asm volatile(
        "mov x0, %0;"\
        "mov x1, %1;"\
        "SVC #1;"
        :
        : "r"((uint64_t) no), "r"((uint64_t) data)
        : "x0","x1");
    __builtin_unreachable ();
}

int wait(err_t * status){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "SVC #2;"\
        "mov %0, x0;"
        : "=r"((uint64_t) ret)
        :  "r"((uint64_t) status)
        :"x0");
    return (int)ret;
}

int send(int target_pid, uint64_t data1, uint64_t data2,
         recv_t * receive_data, bool share_buff){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "mov x1, %2;"\
        "mov x2, %3;"\
        "mov x3, %4;"\
        "mov x4, %5;"\
        "SVC #3;"\
        "mov %0, x0;"
        : "=r"((uint64_t) ret)
        :  "r"((uint64_t) target_pid), "r"(data1), "r"(data2),
         "r"((uint64_t) receive_data), "r"((uint64_t) share_buff)
        :"x0", "x1", "x2", "x3", "x4");
    return (int)ret;
}

int receive(recv_t * receive_data){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "SVC #4;"\
        "mov %0, x0;"
        : "=r"((uint64_t) ret)
        :  "r"((uint64_t) receive_data)
        :"x0");
    return (int)ret;
}

int acknowledge(int return_code, uint64_t data1, uint64_t data2){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "mov x1, %2;"\
        "mov x2, %3;"\
        "SVC #5;"\
        "mov %0, x0;"
        : "=r"((uint64_t) ret)
        :  "r"((uint64_t) return_code), "r"(data1), "r"(data2)
        :"x0", "x1", "x2", "x3", "x4");
    return (int)ret;
}
