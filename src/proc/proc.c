#include "proc.h"
#include "proc_mmu.h"
#include "../libc/uart/uart.h"
#include "../libc/debug/debug.h"
#include "../memory/alloc.h"
#include "../usr/init.h"
#include "../interrupt.h"

/* ASM function deined in proc_asm.s */
__attribute__((__noreturn__))
extern void restore_and_run(uint64_t reg_end, uint64_t pc, uint64_t sp, uint64_t pstate);

/* Warning : do not remove the "static" here o/w it leads to strange behavior */
static system_state sys_state;

uint64_t new_el0_pstate(){
    uint64_t pstate = 0;
    /* Enable exceptions */
    pstate |= MASK(9,6);
    /* Mode = 0 -> EL0 */
    return pstate;
}

proc_descriptor new_proc_descriptor(int pid, int parent_pid, int priority, uint64_t pc){
    proc_descriptor proc;
    proc.pid                  = pid;
    proc.parent_pid           = parent_pid;
    proc.priority             = priority;
    proc.state                = RUNNABLE;
    proc.sched_policy         = DEFAULT;
    proc.mem_conf.initialized = false;
    proc.saved_context.pstate = new_el0_pstate();
    proc.saved_context.sp     = STACK_BEGIN;
    proc.saved_context.pc     = pc;
    /* We kindly initialize the registers to zero */
    for(int i = 0; i < N_REG; i++) proc.saved_context.registers[i] = 0;
    return proc;
}

/* Init the process management system */
void init_proc(){
    uart_info("Beginning proc initialization\r\n");
    /* SIMD insructions should already be enabled at EL0 (they are enabled in switch_from_EL3_to_EL1)*/
    /* For now we are still at EL1 in kernel mode */
    sys_state.curr_pid = 0;
    for(int i = 0; i < MAX_PROC; i++){
        sys_state.procs[i].state = FREE;
    }
    sys_state.procs[0].state = KERNEL;
    /* Backup TTBR0_EL1 for kernel */
    uint64_t ttbr0_el1;
    asm volatile("mrs %0, TTBR0_EL1" : "=r"(ttbr0_el1));
    sys_state.procs[0].mem_conf.ttbr0_el1 = ttbr0_el1;
    /* Create init process (PID 1) */
    /* Don't understand what the hell was happening with this : p was set to zero while gdb says &proc0_main != 0 */
    /* void (*p)() = 42; */
    /* p = &proc0_main; */
    /* uart_debug("p = %d", (int)p); */
    /* -> Workaround :*/
    uint64_t p;
    asm volatile("ldr %0, =proc0_main" : "=r"(p));
    sys_state.procs[1] = new_proc_descriptor(1, 1, 0, p);
    uart_info("Proc initialization done\r\n");
    return;
}


/* TODO : NEON/SIMD registers ?? */
void save_context(uint64_t sp, uint64_t elr_el1, uint64_t pstate, uint64_t handler_sp){
    context * const ctx = &(sys_state.procs[sys_state.curr_pid].saved_context);
    ctx->sp = sp;
    ctx->pc = elr_el1;
    ctx->pstate = pstate;
    for(int i = 0; i < N_REG; i++){
        ctx->registers[i] = AT(handler_sp + (N_REG + 1 - i) * sizeof(uint64_t));
    }
    return;
}

/* Run again process */
/* Assumes MMU, Stack... is already set up */
__attribute__((__noreturn__))
void run_process(const proc_descriptor * proc){
    uart_verbose("Preparing to run process with PID %d with code at address 0x%x\r\n", proc->pid, proc->saved_context.pc);
    assert(proc->pid > 0);      /* Should not be used on kernel */
    assert(proc->state == RUNNABLE);
    assert(proc->mem_conf.initialized);
    sys_state.curr_pid = proc -> pid;
    switch_to_proc_mmu(proc);
    uart_debug("Successfully switched to process MMU\r\n");
    restore_and_run((uint64_t) &(proc->saved_context.registers[N_REG - 1]),
                    proc->saved_context.pc,
                    proc->saved_context.sp,
                    proc->saved_context.pstate);
}

/**** START PROCESS ****/

/* Start a new process (set up MMU, stack...) */
__attribute__((__noreturn__))
int start_process(proc_descriptor * proc){
    assert(!proc->mem_conf.initialized);
    proc->mem_conf.initialized = true;
    /* We expect at least pc to be set */
    /* Here proc param ttbr0 and sp are set */
    set_up_memory_new_proc(proc);
    run_process(proc);
}


int exec_proc(int pid){
    if(sys_state.procs[pid].mem_conf.initialized) run_process(&sys_state.procs[pid]);
    else start_process(&sys_state.procs[pid]);
}



















/* Should not return */
__attribute__((__noreturn__))
void c_el1_svc_aarch64_handler(uint64_t esr_el1){
    uint16_t syscall = (esr_el1 & 0x1ffffff); //get back syscall code
    switch(syscall){
        default:
            uart_error(
                "Error no syscall SVC Aarch64 corresponding to code %d\r\n",
                syscall);
            display_esr_eln_info(esr_el1);
            uart_error("Aborting...\r\n");
            abort();
    }
}








