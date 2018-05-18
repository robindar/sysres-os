#ifndef SYS_H
#define SYS_H
#include "errno.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t data1;
    uint64_t data2;
} recv_t;

#define SYSCALL(code) asm volatile("SVC #" # code "")
int fork(int priority);
__attribute__((__noreturn__))
void exit(errno_t no, errdata_t data);
int wait(err_t * status);
int send(int target_pid, uint64_t data1, uint64_t data2,
         recv_t * receive_data, bool share_buff);
int receive(recv_t * receive_data);
int acknowledge(int return_code, uint64_t data1, uint64_t data2);
#endif
