#include "proc.h"
#include "../memory/mmu.h"
#include "alloc.h"

/* Note :
 * The MMU configuration through TCR_EL1 done in memory/init_mmu.s does not need
 * to be done again for EL0 :
 * TCR_EL1 controls both EL0 and EL1 levels (ARM ARM 2685)
*/


/* TODO : important finish adapting functions from mmu.c -> gloabl cst for lvl2 table and then commit */
void set_up_memory_new_proc(proc_descriptor * proc){
    uart_info(
        "Beginning C MMU initialization for process of PID : %d\r\n", proc->pid);
    /* Allocate level 2 table */
    proc->mem_conf.ttbr0_el1 =
        (uint64_t) kmalloc(N_TABLE_ENTRIES * sizeof(uint64_t));

    /* TODO : improve this -> tables should be allocated on demand : change bind_address */
                /* QUESTION : Alloc with kmalloc or get_new_page ? -> maybe make an "immediate" version of kmalloc to avoid interrupts */
            uint64_t lvl3_tables_address =
                (uint64_t) kmalloc(N_TABLE_ENTRIES * N_TABLE_ENTRIES * sizeof(uint64_t));
    populate_lvl2_table(lvl3_tables_address);
    uint64_t id_paging_size = identity_paging();
    /* Maybe remove the next line later */
    check_identity_paging(id_paging_size);
}
