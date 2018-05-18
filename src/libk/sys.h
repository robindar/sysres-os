#ifndef SYS_H
#define SYS_H
#include "errno.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SYSCALL(code) asm volatile("SVC #" # code "")
int fork(int priority);
__attribute__((__noreturn__))
void exit(errno_t no, errdata_t data);
int wait(err_t * status);
int send(int target_pid, void * send_data, size_t send_size, void * ack_data, size_t ack_size, bool wait);
int receive(void * receive_data, size_t receive_size);
int acknowledge(int return_code, void * ack_data, size_t ack_size);
#endif
