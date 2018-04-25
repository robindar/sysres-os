#include "proc.h"
#include "../memory/mmu.h"
#include "../memory/alloc.h"

/* Note :
 * The MMU configuration through TCR_EL1 done in memory/init_mmu.s does not need
 * to be done again for EL0 :
 * TCR_EL1 controls both EL0 and EL1 levels (ARM ARM 2685)
*/


/* TODO : important finish adapting functions from mmu.c -> gloabl cst for lvl2 table and then commit */
void set_up_memory_new_proc(proc_descriptor * proc){
    /* Should not be used for kernel */
    assert(proc->pid > 0);
    /* Allocate level 2 table */
    uint64_t lvl2_table_address =
        (uint64_t) kmalloc(N_TABLE_ENTRIES * sizeof(uint64_t));

    /* TODO : improve this -> tables should be allocated on demand : change bind_address */
    /* QUESTION : Alloc with kmalloc or get_new_page ? -> maybe make an "immediate" version of kmalloc to avoid interrupts */
    uint64_t lvl3_tables_address =
        (uint64_t) kmalloc(N_TABLE_ENTRIES * N_TABLE_ENTRIES * sizeof(uint64_t));
    c_init_mmu(proc->pid, lvl2_table_address, lvl3_tables_address);
    proc->mem_conf.ttbr0_el1 = (((uint64_t)proc->pid) << 48) | (lvl2_table_address & MASK(47,1));
    proc->saved_context.sp = STACK_BEGIN;
}

/* The code running here is in the kernel, id mapped for every proc */
/* So there should be no pb after we swicth to proc MMU */
void switch_to_proc_mmu(const proc_descriptor * proc){
    asm volatile("msr TTBR0_EL1, %0" : : "r"(proc->mem_conf.ttbr0_el1) : );
    asm volatile("ISB");
    return;
}
