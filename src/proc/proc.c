#include "proc.h"
#include "proc_mmu.h"
#include "../libc/uart/uart.h"
#include "../libc/debug/debug.h"

system_state sys_state;


void init_proc(){
    uart_info("Beginning proc initialization\r\n");
    /* SIMD insructions should already be enabled at EL0 */
    uart_info("Proc initialization done\r\n");
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
    assert(proc->state == RUNNABLE)
    sys_state.curr_pid = proc -> pid;
    switch_to_proc_mmu(proc);
    restore_and_run(&(proc->saved_context.registers[N_REG - 1]),
                    proc->saved_context.pc,
                    proc->saved_context.sp,
                    proc->saved_context.pstate);
}

/**** START PROCESS ****/

uint64_t new_el0_pstate(){
    uint64_t pstate = 0;
    /* Enable exceptions */
    pstate |= MASK(9,6);
    /* Mode = 0 -> EL0 */
    return pstate;
}

/* Start a new process (set up MMU, stack...) */
__attribute__((__noreturn__))
int start_process(proc_descriptor * proc){
    /* Here proc param ttbr0 and sp are set */
    proc->saved_context.pstate = new_el0_pstate();
    run_process(proc);
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








