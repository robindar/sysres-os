#ifndef IO_H
#define IO_H
#include <stdint.h>
#include "../proc/proc.h"
#define IO_BUFF_SIZE 256
typedef struct {
    int code;
    uint64_t data;
    char buff [IO_BUFF_SIZE];
} io_request_t;

typedef struct {
    char buff [IO_BUFF_SIZE];
} io_response_t;

__attribute__((__noreturn__))
void main_io_manager();
#endif
