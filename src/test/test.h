#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdint.h>
#include "../memory/alloc.h"
#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../memory/mmu.h"
#include "../libk/sys.h"

void print_formatting_tests();
void debug_test();
void syscall_test();
void malloc_test();
void matrix_main();
void matrix_main_soft();
void free_test();
void id_syscall_test();
void fork_test1();
void fork_test2();
void kernel_timer_test();
void proc_timer_test();
void timer_test_to_call_at_c_sync_handler();
void chan_test1();
#endif
