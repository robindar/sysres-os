#include "test.h"
#include "../libk/debug.h"
#include "../libk/uart.h"
#include "../libk/io_lib.h"
#include "../usr/io.h"

void print_formatting_tests() {
    uart_debug("Performing printf formatting tests:\r\n");
    uart_debug("\tShould output \"hey\": %s\r\n", "hey");
    uart_debug("\tShould output \"2a\": %x\r\n", 42);
    uart_debug("\tShould output \"101010\": %b\r\n", 42);
    uart_debug("\tShould output \"52\": %o\r\n", 42);
    uart_debug("\tShould output \"-42\": %d\r\n", -42);
    uart_debug("\tShould output 2 ** 32 + 1: %b\r\n", (((uint64_t) 1) << 32) + 1);
    uart_debug("\tShould output \"100000001\": %x\r\n", (((uint64_t) 1) << 32) + 1);
    uart_debug("Done testing printf formatting\r\n");
}

void debug_test() {
    uart_debug("Performing assert test : should fail\r\n");
    assert(0);
}


void syscall_test() {
    uart_debug(
        "Performing SVC test :should fail because there is no syscall with code 1\r\n");
    asm volatile ("svc #0x1"::);
}

void malloc_test() {
    uart_debug("Entering malloc test\r\n");
    uart_debug("Allocating uint64 dynamically\r\n");
    uint64_t * p = (uint64_t *) kmalloc(sizeof(uint64_t));
    uart_debug("Modifying memory at pointer location (should allocate new page)\r\n");
    *p = 42;
    uart_debug("*p = %d (should be 42)\r\n", *p);
    uart_debug("Freeing virtual page\r\n");
    usr_free_virtual_page((uint64_t) p);
    uart_debug("Accessing freed page, should trigger Translation Fault\r\n");
    *p = 43;                    /* Should cause an Tranlsation Fault again */
    uart_debug("Trying kmalloc with an array\r\n");
    char * array = (char *) kmalloc(GRANULE * sizeof(char));
    array[GRANULE - 1] = 42;
    uart_debug("p = 0x%x\r\narray = 0x%x\r\n",p, array);

    /* The following test sequence needs to be manually checked
     * make sure to use compile flag -D MALLOC_VERBOSE
     * And go through the logs to check that the malloc blocks
     *   are properly merged and split
     */
    uart_debug("Testing malloc block merge\r\n");
    uart_debug("Allocating first array (size 10)\r\n");
    uint64_t * p1 = (uint64_t *) kmalloc(10 * sizeof(uint64_t));
    p1[0] = 42;
    uart_debug("Allocating second array (size 20)\r\n");
    uint64_t * p2 = (uint64_t *) kmalloc(20 * sizeof(uint64_t));
    p2[0] = 42;
    uart_debug("Freeing first array\r\n");
    kfree(p1);
    uart_debug("Allocating third array (size 30)\r\n");
    uint64_t * p3 = (uint64_t *) kmalloc(30 * sizeof(uint64_t));
    p3[0] = 42;
    uart_debug("Allocating fourth array (size 40)\r\n");
    uint64_t * p4 = (uint64_t *) kmalloc(40 * sizeof(uint64_t));
    p4[0] = 42;
    uart_debug("Freeing third array\r\n");
    kfree(p3);
    uart_debug("Freeing second array\r\n");
    kfree(p2);
    uart_debug("Allocating fifth array (size 50)\r\n");
    uint64_t * p5 = (uint64_t *) kmalloc(50 * sizeof(uint64_t));
    p5[0] = 42;
    uart_debug("Freeing fourth array\r\n");
    kfree(p4);
    uart_debug("Freeing fifth array\r\n");
    kfree(p5);

    kfree(p);
    kfree(array);
    uart_debug("Done testing malloc\r\n");
}

void a_page_s_file(){
    uart_printf("Entering malloc test\r\n");
    uart_printf("Allocating uint64 dynamically\r\n");
    uint64_t * p = (uint64_t *) kmalloc(sizeof(uint64_t));
    uart_printf("Modifying memory at pointer location (should allocate new page)\r\n");
    *p = 42;
    uart_printf("*p = %d (should be 42)\r\n", *p);
}

void random_test(){
    for(int i = 0; i < 1000; i++){
        uart_printf("-- %d ", random(10000));
    }
    unsigned int law[10];
    for(int i = 0; i < 10; i++) law[i] = i;
    for(int i = 0; i < 10; i++){
        uart_debug("Rand according to law between 0 and 10: %d\r\n", random_law(law, 10));;
    }
}

void shutdown_test(){
    BEGIN_TEST();
    int ret = fork(14);
    assert(ret != -1);
    if(ret == 0){
        /* Child */
        int code = 42;
        ret = send(INIT_PID, &code, sizeof(int), NULL, 0, true);
        assert(ret == -1);
        shutdown();
    }
    END_TEST();
    return;
}

void test_priviledged_get_string(){
    char buff[256];
    while(1){
        uart_printf(":");
        uart_get_string(buff, 256);
        uart_printf("Got: %s\r\n", buff);
    }
}

void io_simple_test(){
    BEGIN_TEST();
    io_putc('@');
    END_TEST();
}

void print_io_formatting_tests() {
    io_printf("Performing printf formatting tests:\r\n");
    io_printf("\tShould output \"hey\": %s\r\n", "hey");
    io_printf("\tShould output \"2a\": %x\r\n", 42);
    io_printf("\tShould output \"101010\": %b\r\n", 42);
    io_printf("\tShould output \"52\": %o\r\n", 42);
    io_printf("\tShould output \"-42\": %d\r\n", -42);
    io_printf("\tShould output 2 ** 32 + 1: %b\r\n", (((uint64_t) 1) << 32) + 1);
    io_printf("\tShould output \"100000001\": %x\r\n", (((uint64_t) 1) << 32) + 1);
    io_printf("Done testing printf formatting\r\n");
}

void test_io_get_string(int n){
    char buff[256];
    for(int i = 0; i < n; i++){
        io_printf(":");
        io_get_string(buff, 256);
        io_printf("Got: %s\r\n", buff);
    }
}

void time_send(){
    int code = 1;
    uint64_t before, after, sum = 0;
    int i, status, n = 100000;
    print_reg(CNTFRQ_EL0);
    uart_printf("Beginning send benchmark\r\n");
    for(i = 0; i < n; i++){
        before = time();
        status = send(INIT_PID, &code, sizeof(int),NULL,0, true);
        after = time();
        sum += after - before;
        assert(status == 0);
    }
    uart_printf("On %d tests\r\naverage = %d clock ticks\r\naverage = %d nano-seconds\r\n", n, sum/n, 12*sum/(n  * 10));
}
