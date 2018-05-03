#include "proc.h"
#include "../memory/mmu.h"

/* Note :
 * The MMU configuration through TCR_EL1 done in memory/init_mmu.s does not need
 * to be done again for EL0 :
 * TCR_EL1 controls both EL0 and EL1 levels (ARM ARM 2685)
*/
__attribute__((__noreturn__))
extern void restore_and_run(uint64_t reg_addr_end, uint64_t pc, uint64_t sp, uint64_t pstate);

/* TODO : important finish adapting functions from mmu.c -> gloabl cst for lvl2 table and then commit */
void set_up_memory_new_proc(proc_descriptor * proc){
    uint64_t lvl2_table_address = c_init_mmu(proc->pid);
    proc->mem_conf.ttbr0_el1 = (((uint64_t)proc->pid) << 48) | (lvl2_table_address & MASK(47,1));
    uart_verbose(
        "MMU set up for PID : %d\r\nASID : %d\r\nlvl2_table_address : 0x%x\r\n",
        proc->pid,
        get_ASID_from_TTBR0(proc->mem_conf.ttbr0_el1),
        get_lvl2_table_address_from_TTBR0(proc->mem_conf.ttbr0_el1 & MASK(47, 1)));
    /* TODO : malloc for user and init_alloc */
    return;
}

/* The code running here is in the kernel, id mapped for every proc */
/* So there should be no pb after we swicth to proc MMU */
/* Thanks to the ASID we don't need to clean the TLB (ARMv8-A Address Translation) */
__attribute__((__noreturn__))
void switch_to_proc(const proc_descriptor * proc){
    /* Do not call anything here */
    asm volatile("msr TTBR0_EL1, %0" : : "r"(proc->mem_conf.ttbr0_el1) : );
    asm volatile("ISB");
    uart_info("Successfully switched to process MMU\r\n");
    uart_debug("Block 0x7000 permission : 0b%b\r\n", get_page_permission(0x7000));
    set_lvl2_address_from_TTBR0_EL1();
    /* Things to emove after debugging : */
    /* - End of free_virtual_page */
    /* - Uart_verbose in data_abort handler */
    free_virtual_page(0x7000);
    delay((1 << 26));
    uint64_t data = *((uint64_t*)0x7000);
    uart_debug("hey = %x\r\n", data);
    uart_debug("Shouldn't happen\r\n");
    restore_and_run((uint64_t) &(proc->saved_context.registers[N_REG - 1]),
                    proc->saved_context.pc,
                    proc->saved_context.sp,
                    proc->saved_context.pstate);
}
