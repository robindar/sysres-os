#include "proc.h"
#include "../memory/mmu.h"

/* Note :
 * The MMU configuration through TCR_EL1 done in memory/init_mmu.s does not need
 * to be done again for EL0 :
 * TCR_EL1 controls both EL0 and EL1 levels (ARM ARM 2685)
*/


/* TODO : important finish adapting functions from mmu.c -> gloabl cst for lvl2 table and then commit */
void set_up_memory_new_proc(proc_descriptor * proc){;
    uint64_t lvl2_table_address = c_init_mmu(proc->pid);
    proc->mem_conf.ttbr0_el1 = (((uint64_t)proc->pid) << 48) | (lvl2_table_address & MASK(47,1));
    /* TODO : malloc for user and int_alloc */
    return;
}

/* The code running here is in the kernel, id mapped for every proc */
/* So there should be no pb after we swicth to proc MMU */
/* Thanks to the ASID we don't need to clean the TLB (ARMv8-A Address Translation) */
void switch_to_proc_mmu(const proc_descriptor * proc){
    asm volatile("msr TTBR0_EL1, %0" : : "r"(proc->mem_conf.ttbr0_el1) : );
    asm volatile("ISB");
    return;
}
