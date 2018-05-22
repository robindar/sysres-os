#ifndef ALLOC_H
#define ALLOC_H
#include <stddef.h>
#include <stdint.h>
#include "mmu.h"
#include "../libk/string.h"
#include "../libk/uart.h"
#include "../interrupt/timer.h"

/* Stack begins at #3F200000 see boot.s */
#define STACK_BEGIN  GPIO_BASE
#define STACK_END    (STACK_BEGIN - 2 * GRANULE)
#define STACK_PAGE_1 (STACK_BEGIN - 1 * GRANULE)
#define STACK_PAGE_2 (STACK_BEGIN - 2 * GRANULE)
#define PAGE_RES_1   (STACK_END   - 1 * GRANULE)
#define PAGE_RES_2   (STACK_END   - 2 * GRANULE)
#define HEAP_MAX     TIMER_IRQ_PAGE

void * ksbrk(int increment);
void init_alloc();
void * kmalloc(size_t size);
void kfree(void * p);
/* Internal state of alloc related funcs */
uint64_t get_heap_begin();
uint64_t get_end_offset();
void * get_global_base();
void set_heap_begin(uint64_t val);
void set_end_offset(int val);
void set_global_base(void * val);
int usr_free_virtual_page(uint64_t virtual_address);
#endif
