#include "test.h"

/* Must be executed in Kernel mode */
void kernel_timer_test(){
    exec_proc(1);
}

/* Must be put in main_init proc */
void proc_timer_test(){
    uart_debug("Beginning timer test\r\n");
    matrix_main();
    while(1){
        uart_printf(".");
        delay(1 << 20);
    }
}

/* To make it more easy to follow you should call this at the very beginning of */
/* c_sync_handler */
/* and play a bit with the size to have more data aborts */
void timer_test_to_call_at_c_sync_handler(){
    print_timer_status();
    delay(1 << 25);
}
