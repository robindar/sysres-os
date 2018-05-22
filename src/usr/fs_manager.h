#ifndef FS_MANAGER_H
#define FS_MANAGER_H
#include <stdint.h>
#include "../proc/proc.h"
#include "io.h"
typedef struct {
    int code;
    int file_descriptor;
    uint64_t data1;
    uint64_t data2;
    char buff1[IO_BUFF_SIZE];
    char buff2[IO_BUFF_SIZE];
} fs_request_t;

typedef struct {
    char buff [IO_BUFF_SIZE];
} fs_response_t;

__attribute__((__noreturn__))
void main_fs_manager();
#endif
