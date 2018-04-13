#ifndef ALLOC_H
#define ALLOC_H
#include <stdint.h>
#include <stddef.h>
#include "mmu.h"
#include "../libc/uart/uart.h"

void * ksbrk(int increment);
void init_alloc();
void * kmalloc(size_t size);
void kfree(void * p);

#endif
