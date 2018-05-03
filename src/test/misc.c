#include "test.h"
#include "../libc/uart/uart.h"
#include "../libc/debug/debug.h"

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
    free_virtual_page((uint64_t) p);
    uart_debug("Accessing freed page, should trigger Translation Fault\r\n");
    *p = 43;                    /* Should cause an Tranlsation Fault again */
    uart_debug("Trying kmalloc with an array\r\n");
    char * array = (char *) kmalloc(GRANULE * sizeof(char));
    array[GRANULE - 1] = 42;
    uart_debug("p = 0x%x\r\narray = 0x%x\r\n",p, array);
    kfree(p);
    kfree(array);
    uart_debug("Done testing malloc\r\n");
}
