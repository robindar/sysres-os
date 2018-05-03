#ifndef PROC_MMU_H
#define PROC_MMU_H
#include "proc.h"
void set_up_memory_new_proc(proc_descriptor * proc);
__attribute__((__noreturn__))
void switch_to_proc(const proc_descriptor * proc);
#endif
