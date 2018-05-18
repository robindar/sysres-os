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

int send(int target_pid, void * send_data, size_t send_size, void * ack_data, size_t ack_size, bool wait){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "mov x1, %2;"\
        "mov x2, %3;"\
        "mov x3, %4;"\
        "mov x4, %5;"\
        "mov x5, %6;"\
        "SVC #3;"\
        "mov %0, x0;"
        : "=r"((uint64_t) ret)
        :  "r"((uint64_t) target_pid), "r"((uint64_t) send_data),
           "r"((uint64_t) send_size),  "r"((uint64_t) ack_data),
           "r"((uint64_t) ack_size),   "r"((uint64_t) wait)
        :"x0", "x1", "x2", "x3", "x4", "x5");
    return (int)ret;
}

int receive(void * receive_data, size_t receive_size){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "mov x1, %2;"\
        "SVC #4;"\
        "mov %0, x0;"
        : "=r"((uint64_t) ret)
        :  "r"((uint64_t) receive_data), "r"((uint64_t) receive_size)
        :"x0", "x1");
    return (int)ret;
}

int acknowledge(int return_code, void * ack_data, size_t ack_size){
    uint64_t ret;
    asm volatile(
        "mov x0, %1;"\
        "mov x1, %2;"\
        "mov x2, %3;"\
        "SVC #5;"\
        "mov %0, x0;"
        : "=r"((uint64_t) ret)
        :  "r"((uint64_t) return_code), "r"((uint64_t) ack_data),
           "r"((uint64_t) ack_size)
        :"x0", "x1", "x2");
    return (int)ret;
}
