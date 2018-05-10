#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdint.h>
#include "../memory/alloc.h"
#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../memory/mmu.h"
#include "../usr/libc.h"

void print_formatting_tests();
void debug_test();
void syscall_test();
void malloc_test();
void matrix_main();
void free_test();
void id_syscall_test();
#endif
