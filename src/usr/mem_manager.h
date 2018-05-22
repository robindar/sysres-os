#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H
#include <stdint.h>

typedef struct{
    int       code;
    uint64_t  data;
} mem_request_t;

__attribute__((__noreturn__))
void main_mem_manager();

#endif
