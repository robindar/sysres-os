#ifndef SYS_H
#define SYS_H
#include "errno.h"
#define SYSCALL(code) asm volatile("SVC #" # code "")
int fork(int priority);
__attribute__((__noreturn__))
void exit(errno_t no, errdata_t data);
int wait(err_t * status);
#endif
