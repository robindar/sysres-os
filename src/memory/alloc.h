#ifndef ALLOC_H
#define ALLOC_H
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include "mmu.h"
#include "../libc/string.h"
#include "../libc/uart/uart.h"

/* Stack begins at #3F200000 see boot.s */
#define STACK_BEGIN GPIO_BASE
#define STACK_END   (STACK_BEGIN - 2 * GRANULE)

void * ksbrk(int increment);
void init_alloc();
void * kmalloc(size_t size);
void kfree(void * p);
uint64_t get_heap_begin();
uint64_t get_end_offset();

#endif
