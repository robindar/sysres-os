#ifndef TEST_H
#define TEST_H

#include "../memory/alloc.h"
#include "../libc/debug/debug.h"
#include "../libc/uart/uart.h"
#include "../memory/mmu.h"
#include <stdint.h>
#include <stdbool.h>

void print_formatting_tests();
void debug_test();
void syscall_test();
void malloc_test();
void matrix_main();
void free_test();

#endif
