#include "sys.h"
#include <stdint.h>

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
