#include "proc.h"
#include "proc_mmu.h"
#include "../interrupt/interrupt.h"
#include "../libk/debug.h"
#include "../libk/misc.h"
#include "../libk/uart.h"
#include "../memory/alloc.h"
#include "../usr/init.h"
#include "../libk/errno.h"

/**** INIT *****/
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
    proc.initialized          = false;
    proc.saved_context.pstate = new_el0_pstate();
    proc.saved_context.sp     = STACK_BEGIN;
    proc.saved_context.pc     = pc;
    /* We kindly initialize the registers to zero */
    for(int i = 0; i < N_REG; i++) proc.saved_context.registers[i] = 0;
    return proc;
}

#ifdef PROC_VERBOSE
#define proc_verbose(...) uart_verbose(__VA_ARGS__)
#else
#define proc_verbose(...) ((void) 0)
#endif


void print_proc_descriptor(const proc_descriptor * proc){
    uart_verbose(
        "PID : %d\r\n"
        "Parent PID : %d\r\n"
        "Priority : %d\r\n"
        "State : %d\r\n"
        "Initialized : %s\r\n"
        "Errno : %d\r\n",
        proc->pid, proc->parent_pid, proc->priority,
        proc->state, proc->initialized ? "Yes" : "No",
        proc->errno);
    uart_verbose(
        "pc : 0x%x\r\n"
        "sp : 0x%x\r\n"
        "pstate : 0x%x\r\n",
        proc->saved_context.pc, proc->saved_context.sp,
        proc->saved_context.pstate);
    for(int i = 0; 4*i + 3 < N_REG; i++){
        uart_verbose("X%d : 0x%x  X%d : 0x%x  X%d : 0x%x  X%d : 0x%x\r\n",
                     4*i, proc->saved_context.registers[4*i],
                     4*i + 1, proc->saved_context.registers[4*i+1],
                     4*i + 2, proc->saved_context.registers[4*i+2],
                     4*i + 3, proc->saved_context.registers[4*i+3]);
    }
    uart_verbose("X%d : 0x%x  X%d : 0x%x  X%d : 0x%x\r\n",
                 28, proc->saved_context.registers[28],
                 29, proc->saved_context.registers[29],
                 30, proc->saved_context.registers[30]);
    uart_verbose(
        "TTBR0_EL1 : 0x%x\r\n"
        "heap_begin : 0x%x\r\n"
        "end_offset : 0x%x\r\n"
        "global_base : 0x%x\r\n",
        proc->mem_conf.ttbr0_el1, proc->mem_conf.heap_begin,
        proc->mem_conf.end_offset,
        (uint64_t)proc->mem_conf.global_base);
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
    /* p = proc0_main; */
    /* uart_debug("p = %d", (int)p); */
    /* -> Workaround :*/
    uint64_t p;
    asm volatile("ldr %0, =proc0_main" : "=r"(p));
    sys_state.procs[1] = new_proc_descriptor(1, 1, 0, p);
    assert(sys_state.procs[1].pid == 1);
    uart_info("Proc initialization done\r\n");
    return;
}



/****  CONTEXT  ****/
void save_errno(proc_descriptor * proc){
    proc->errno = errno;
    return;
}

void restore_errno(const proc_descriptor * proc){
    errno = proc->errno;
}

void save_alloc_conf(proc_descriptor * proc){
    /* No need to touch mem_conf.ttbr0_el1 : it is cst for a given proc */
    proc->mem_conf.heap_begin = get_heap_begin();
    proc->mem_conf.end_offset = get_end_offset();
    proc->mem_conf.global_base = get_global_base();
    return;
}

/* WARNING : kernel memory alloc functions mustn't be called after this*/
void restore_alloc_conf(const proc_descriptor * proc){
    set_heap_begin(proc->mem_conf.heap_begin);
    set_end_offset(proc->mem_conf.end_offset);
    set_global_base(proc->mem_conf.global_base);
    return;
}

/* TODO : NEON/SIMD registers ?? */
/* Warning :
   may be confusing, but it does more thing than its counterpart restore_context */
/* We are still with the proc MMU and SP */
void save_context(uint64_t sp, uint64_t elr_el1, uint64_t pstate, uint64_t handler_sp){
    context * const ctx = &(sys_state.procs[sys_state.curr_pid].saved_context);
    ctx->sp = sp;
    ctx->pc = elr_el1;
    ctx->pstate = pstate;
    for(int i = 0; i < N_REG/2; i++){
        ctx->registers[2*i+1] = AT(handler_sp + (N_REG - 2*i  ) * sizeof(uint64_t));
        ctx->registers[2*i]   = AT(handler_sp + (N_REG - 2*i-1) * sizeof(uint64_t));
    }
    ctx->registers[30] = AT(handler_sp + sizeof(uint64_t));
    save_alloc_conf(&sys_state.procs[sys_state.curr_pid]);
    save_errno((&sys_state.procs[sys_state.curr_pid]));
    /* We are in kernel mode now */
    /* set_lvl2_address_from_pid(0); */
    /* Indeed, setting lvl2_addr should be done only in mmu.c */
    sys_state.last_pid = sys_state.curr_pid;
    sys_state.curr_pid = 0;
    return;
}

/* Run again process */
/* Assumes MMU, Stack... is already set up */
__attribute__((__noreturn__))
void run_process(proc_descriptor * proc){
    uart_verbose("Preparing to run process with PID %d with code at address 0x%x\r\n", proc->pid, proc->saved_context.pc);
    #ifdef PROC_VERBOSE
    print_proc_descriptor(proc);
    #endif
    assert(proc->pid > 0);      /* Should not be used on kernel */
    assert(proc->state == RUNNABLE);
    /* Must be in kernel mode */
    assert(sys_state.curr_pid == 0);
    /* Backup alloc info for kernel */
    save_alloc_conf(&sys_state.procs[0]);
     if(!proc->initialized){
         /* Init proc MMU */
        set_up_memory_new_proc(proc);
    }
    sys_state.curr_pid = proc -> pid;
    switch_to_proc(proc);
}

/**** START PROCESS ****/

/* Kernel function only */
int exec_proc(int pid){
    run_process(&sys_state.procs[pid]);
}



/**** SYSCALLS ****/
/* Should not return */
__attribute__((__noreturn__))
void c_el1_svc_aarch64_handler(uint64_t esr_el1){
    uart_verbose("C EL1 SVC AARCH64 Handler called\r\n");
    #ifdef PROC_VERBOSE
    print_proc_descriptor(&sys_state.procs[sys_state.last_pid]);
    #endif
    uint16_t syscall = (esr_el1 & MASK(15,0));
    //get back syscall code (ARM ARM 2453 for encoding)
    switch(syscall){
    case 100:
        /* Halt syscall (halt cannot be executed at EL0) */
        uart_verbose("Syscall code 100 : Halt\r\n");
        halt();
        break;
    case 101:
        /* Test syscall : does nothing */
        uart_verbose("Syscall code 101 : Test id\r\n");
        run_process(&sys_state.procs[sys_state.last_pid]);
        break;
    default:
        uart_error(
            "Error no syscall SVC Aarch64 corresponding to code %d\r\n",
            syscall);
        display_esr_eln_info(esr_el1);
        uart_error("Aborting...\r\n");
        abort();
    }
}

/****  SYS_STATE book-keeping for other files *****/
/* Warning : ranslation_fault_handler uses this (and free may ine the future)*/
uint64_t get_lvl2_address_from_sys_state(int pid){
    uint64_t lvl2_address;
    lvl2_address = sys_state.procs[pid].mem_conf.ttbr0_el1;
    lvl2_address &= MASK(47,1);
    return lvl2_address;
}

int get_curr_pid(){
    return sys_state.curr_pid;
}
