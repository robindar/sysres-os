#include "proc.h"
#include "../memory/mmu.h"

/* Note :
 * The MMU configuration through TCR_EL1 done in memory/init_mmu.s does not need
 * to be done again for EL0 :
 * TCR_EL1 controls both EL0 and EL1 levels (ARM ARM 2685)
*/

/* Defined in proc_asm.s */
__attribute__((__noreturn__))
extern void restore_and_run(uint64_t reg_addr_end, uint64_t pc, uint64_t sp, uint64_t pstate, uint64_t ttbr0_el1);

void set_up_memory_new_proc(proc_descriptor * proc){
    uint64_t lvl2_table_address = c_init_mmu(proc->pid);
    proc->mem_conf.ttbr0_el1 = (((uint64_t)proc->pid) << 48) | (lvl2_table_address & MASK(47,1));
    uart_verbose(
        "MMU set up for PID : %d\r\nASID : %d\r\nlvl2_table_address : 0x%x\r\n",
        proc->pid,
        get_ASID_from_TTBR0(proc->mem_conf.ttbr0_el1),
        get_lvl2_table_address_from_TTBR0(proc->mem_conf.ttbr0_el1 & MASK(47, 1)));
    return;
}

/* The code running here is in the kernel, id mapped for every proc */
/* So there should be no major pb after we swicth to proc MMU */
/* However the stack is completly anhilited so don't return ! */
/* Thanks to the ASID we don't need to clean the TLB (ARMv8-A Address Translation) */
__attribute__((__noreturn__))
void switch_to_proc(proc_descriptor * proc){
    /* As data is id mapped, we can set global var now */
    restore_alloc_conf(proc);
    restore_errno(proc);
    proc->initialized = true;
    if(proc->sched_conf.preempt)
        start_countdown(proc->sched_conf.time_left);
    /* we restore sctrl_el1 here */
    asm volatile("msr SCTLR_EL1, %0" : : "r"(proc->saved_context.sctlr_el1));
    /* We are using x0-x7 to pass parameters */
    /* uart_debug("PC: 0x%x\r\n", proc->saved_context.pc); */
    /* asm volatile("msr ELR_EL1, %0"::"r"(proc->saved_context.pc)); */
    restore_and_run(
        (uint64_t) &(proc->saved_context.registers[N_REG - 1]),
        proc->saved_context.pc,
        proc->saved_context.sp,
        proc->saved_context.pstate,
        proc->mem_conf.ttbr0_el1
        );
}
