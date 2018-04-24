#include "proc.h"

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
        ctx->registers[i] = AT(sp + (N_REG + 1 - i) * sizeof(uint64_t))
    }
    return;
}

/* Run again process */
/* Assumes MMU, Stack... is already set up */
__attribute__((__noreturn__))
void run_again_process(const proc_descriptor * proc){
    sys_state.curr_pid = proc -> pid;
    restore_and_run(&(proc->saved_context.registers[N_REG - 1]),
                    proc->saved_context.pc,
                    proc->saved_context.sp,
                    proc->saved_context.pstate);
}

/**** START PROCESS ****/

int start_process(const proc_descriptor * proc){
    
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








