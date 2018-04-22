#include "alloc.h"

/* Stack begins at #3F200000 see boot.s */
#define STACK_BEGIN GPIO_BASE


//Note : we thank GCC who was kindly putting this variable at the same address as physical memeory map (without static) and thus init_alloc was actually modifying physical memory map for our grestest pleasure
static uint64_t heap_begin;
static int end_offset;

void init_alloc(){
    uart_info("Init Alloc...\r\n");
    asm volatile ("ldr %0, =__end" : "=r"(heap_begin) : :);
    end_offset = 0;

    /* For now : half the meory for the stack, half for the heap */
    uart_verbose("Invalid page to separate stack/heap set at 0x%x\r\n",(STACK_BEGIN + heap_begin)/2);
    //set_invalid_page((STACK_BEGIN + heap_begin)/2);
    uart_info("Init Alloc done\r\n");
    return;
}

void * ksbrk(int increment) {
    /* heap_bin + end_offset is the address of the first byte non allocated */
    uart_verbose("ksbrk called with increment : %d and end_offset : 0x%x\r\n",increment, end_offset);
    int res = end_offset + heap_begin;
    end_offset += increment;
    if(increment < 0){
        int nb_pages_to_free = (res / GRANULE) - ((res + increment) / GRANULE) + !(res % GRANULE == 0);
        for(int i = 0; i < nb_pages_to_free; i ++){
            free_virtual_page(res - i * GRANULE);
        }
        res += increment;
    }
    return (void *) ((uint64_t )res);
}

uint64_t get_heap_begin(){
    return heap_begin;
}

uint64_t get_end_offset(){
    return end_offset;
}

void * kmalloc(size_t size){
    return ksbrk(size);
}

void kfree(void * p){
    (void) p;
}
