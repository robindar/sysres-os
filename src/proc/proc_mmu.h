#ifndef PROC_MMU_H
#define PROC_MMU_H
#include "proc.h"
void set_up_memory_new_proc(proc_descriptor * proc);
void switch_to_proc_mmu(const proc_descriptor * proc);
#endif
