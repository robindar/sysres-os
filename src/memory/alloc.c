#include "alloc.h"

/* Stack begins at #3F200000 see boot.s */
#define STACK_BEGIN GPIO_BASE


//Note : we thank GCC who was kindly putting this variable at the same address as physical memeory map (without static) and thus init_alloc was actually modifying physical memory map for our grestest pleasure
static uint64_t heap_begin;

void init_alloc(){
    uart_info("Init Alloc...\r\n");
    asm volatile ("ldr %0, =__end" : "=r"(heap_begin) : :);

    /* For now : half the meory for the stack, half for the heap */
    //set_invalid_page((STACK_BEGIN + heap_begin)/2);
    uart_info("Init Alloc done\r\n");
    return;
}

void * ksbrk(int increment) {
    static uint64_t end_offset = 0;
    uint64_t res = end_offset + heap_begin;
    end_offset += (uint64_t) increment;
    if(increment < 0){
        uint64_t nb_pages_to_free = ((res -increment) / GRANULE) - res / GRANULE;
        for(uint64_t i = 1; i <= nb_pages_to_free; i ++){
            free_virtual_page(res + i * GRANULE);
        }
    }
    return (void *) res;
}

void * kmalloc(size_t size){
    return ksbrk(size);
}

void kfree(void * p){
    (void) p;
}
