#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdint.h>
#include "../memory/alloc.h"
#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../memory/mmu.h"
#include "../libk/sys.h"

#define BEGIN_TEST() uart_verbose("Beginning %s\r\n", __func__);
#define END_TEST()   uart_verbose("Done %s\r\n",      __func__);

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
void fork_test2bis();
void kernel_timer_test();
void proc_timer_test();
void timer_test_to_call_at_c_sync_handler();
void chan_test1();
void chan_test2();
void random_test();
void fork_test3();
void sched_test1();
void fork_test4();
void fork_test4bis();
void shutdown_test();
void test_copy_and_write();
void test_priviledged_get_string();
void print_io_formatting_tests();
void test_io_get_string();
void io_simple_test();
void fs_test1();
void time_send();
void a_page_s_file();
#endif
