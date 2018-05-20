#include "proc.h"
#include "proc_mmu.h"
#include "../interrupt/interrupt.h"
#include "../libk/debug.h"
#include "../libk/misc.h"
#include "../libk/uart.h"
#include "../memory/alloc.h"
#include "../usr/init.h"
#include "../libk/errno.h"
#include <stddef.h>

#define PROC_VERBOSE

/**** INIT *****/
static system_state sys_state;

uint64_t new_el0_pstate(){
    uint64_t pstate = 0;
    /* Enable exceptions : leave DAIF at zero */
    /* Mode = 0 -> EL0 */
    return pstate;
}

proc_descriptor new_proc_descriptor(int pid, int parent_pid, int priority, void (*p)()){
    proc_descriptor proc      = {0};
    proc.pid                  = pid;
    proc.parent_pid           = parent_pid;
    proc.priority             = priority;
    proc.state                = FREE;
    proc.sched_conf.time_left = QUANTUM;
    proc.sched_conf.preempt   = false; /* Temp */
    proc.initialized          = false;
    proc.saved_context.pstate = new_el0_pstate();
    proc.saved_context.sp     = STACK_BEGIN;
    proc.saved_context.pc     = (uint64_t) p;
    proc.err.no               = OK;
    proc.err.data             = 0;
    proc.child                = NULL;
    proc.sender_data.acknowledged = true;
    proc.buffer.write_addr        = 0;
    proc.buffer.used_size         = 0;
    /* We kindly initialize the registers to zero */
    for(int i = 0; i < N_REG; i++) proc.saved_context.registers[i] = 0;
    return proc;
}

void make_child_proc_descriptor(proc_descriptor * child, proc_descriptor * parent, int pid, int priority){
    /* Redundant but to make sure everything is initilized at it should */
    *child = new_proc_descriptor(pid, parent->pid, priority,
                                 (void (*)()) parent->saved_context.pc);
    child->sched_conf.preempt   = parent->sched_conf.preempt;
    child->saved_context.pstate = parent->saved_context.pstate;
    child->saved_context.sp     = parent->saved_context.sp;
    child->saved_context.pc     = parent->saved_context.pc;
    child->err                  = parent->err;
    /* Ack are still for the parent */
    child->sender_data.acknowledged = true;
    for(int i = 1; i < N_REG; i++)
        child->saved_context.registers[i] = parent->saved_context.registers[i];
    return;
}

#ifdef PROC_VERBOSE
#define proc_verbose(...) uart_verbose(__VA_ARGS__)
#else
#define proc_verbose(...) ((void) 0)
#endif


void print_proc_descriptor(const proc_descriptor * proc){
    #ifdef PROC_VERY_VERBOSE
    uart_verbose(
        "PID : %d\r\n"
        "Parent PID : %d\r\n"
        "Priority : %d\r\n"
        "State : %d\r\n"
        "Initialized : %s\r\n"
        "Err.no : %d\r\n",
        "Err.descr : %d\r\n",
        "Preemptible : %d\r\n",
        "Time left : %ds\r\n",
        proc->pid, proc->parent_pid, proc->priority,
        proc->state, proc->initialized ? "Yes" : "No",
        proc->err.no, proc->err.data,
        proc->sched_conf.preempt ? "Yes" : "No",
        proc->sched_conf.time_left * 1000 / SECOND);
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
    #else
    (void) proc;
    #endif
}

void print_prio_lists(){
    proc_descriptor * proc;
    uart_verbose("%d runnable processes\r\n", sys_state.n_runnable_procs);
    for(int prio = 0; prio <= MAX_PRIO; prio ++){
        if(sys_state.prio_proc_n[prio] > 0)
            uart_verbose("%d processes of priority %d: ", sys_state.prio_proc_n[prio], prio);
        proc = sys_state.prio_proc[prio];
        if(proc != NULL){
            do {
                assert(proc->priority == prio);
                assert(proc->state == RUNNABLE);
                uart_printf("%d, ", proc->pid);
                proc = proc->next_same_prio;
            } while(proc != sys_state.prio_proc[prio]);
            uart_printf("\r\n");
        }
    }
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
    change_state(&sys_state.procs[0], KERNEL);
    /* Backup TTBR0_EL1 for kernel */
    uint64_t ttbr0_el1;
    asm volatile("mrs %0, TTBR0_EL1" : "=r"(ttbr0_el1));
    sys_state.procs[0].mem_conf.ttbr0_el1 = ttbr0_el1;
    /* Create init process (PID 1) */
    sys_state.procs[1] = new_proc_descriptor(1, 1, 14, main_init);
    change_state(&sys_state.procs[1],RUNNABLE);
    /* Initialize init proc */
    set_up_memory_new_proc(&sys_state.procs[1]);
    save_alloc_conf(&sys_state.procs[0]);
    init_alloc();
    save_alloc_conf(&sys_state.procs[1]);
    restore_alloc_conf(&sys_state.procs[0]);
    sys_state.procs[1].initialized = true;
    uart_info("Proc initialization done\r\n");
    return;
}



/****  CONTEXT  ****/
void save_errno(proc_descriptor * proc){
    proc->err = err;
    return;
}

void restore_errno(const proc_descriptor * proc){
    err = proc->err;
    return;
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

/* Backup buffer for syscall */
void back_up_buff_svc(proc_descriptor * proc,  uint64_t svc_code){
    void * addr = (void *) proc->saved_context.registers[1];
    size_t size = (size_t) proc->saved_context.registers[2];
    if((svc_code == 3 || svc_code == 5) && (addr != 0 && size > 0)){
        proc->buffer.used_size = size;
        memmove((void *) proc->buffer.buff, addr,
                size < BUFF_SIZE ? size : BUFF_SIZE);
    }
    return;
}

void write_buff_svc(){
    proc_descriptor * proc = &sys_state.procs[sys_state.curr_pid];
    if(proc->buffer.write_addr != 0){
        memmove((void *) proc->buffer.write_addr, (void *) proc->buffer.buff,
                proc->buffer.used_size);
    }
    proc->buffer.write_addr = 0;
    proc->buffer.used_size  = 0;
    return;
}

/* TODO : NEON/SIMD registers ?? */
/* Warning :
   may be confusing, but it does more thing than its counterpart restore_context */
/* We are still with the proc MMU and SP */
void save_context(uint64_t sp, uint64_t elr_el1, uint64_t pstate, uint64_t handler_sp, uint64_t is_timer_irq, uint64_t esr_el1){
    sys_state.procs[sys_state.curr_pid].sched_conf.time_left =
        is_timer_irq ? 0 : get_curr_timer_value();
    clear_ack_timer_irq();   /* Stop timer */
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
    if(is_timer_irq == 0)
        back_up_buff_svc(&sys_state.procs[sys_state.curr_pid],
                         (esr_el1 & MASK(15,0)));
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
    #ifdef PROC_VERBOSE
    uart_verbose("Preparing to run process with PID %d with code at address 0x%x\r\n", proc->pid, proc->saved_context.pc);
    print_proc_descriptor(proc);
    #endif
    /* Should not be used on kernel */
    assert(proc->pid > 0);
    assert(proc->state == RUNNABLE);
    /* Make sure it has time to run */
    assert((!proc->sched_conf.preempt) || proc->sched_conf.time_left > 0);
    /* Must be in kernel mode */
    assert(sys_state.curr_pid == 0);
    /* Initialization is no more done here */
    assert(proc->initialized);
    /* Backup alloc info for kernel */
    save_alloc_conf(&sys_state.procs[0]);
    sys_state.curr_pid = proc -> pid;
    switch_to_proc(proc);
}

/**** START PROCESS ****/

/* Kernel function only */
__attribute__((__noreturn__))
int exec_proc(int pid){
    run_process(&sys_state.procs[pid]);
}


/**** PROCESS UTILITY****/
/* Returns -1 if no slot available */
int find_free_proc(){
    for(int i = 2; i < MAX_PROC; i++)
        if(sys_state.procs[i].state == FREE) return i;
    return -1;
}

void change_state(proc_descriptor * proc, enum proc_state new_state){
    int prio = proc->priority;
    if(proc->state == RUNNABLE && new_state != RUNNABLE){
        proc_descriptor * next = proc->next_same_prio;
        proc_descriptor * prev = proc->prev_same_prio;
        if(sys_state.prio_proc[prio] == proc) sys_state.prio_proc[prio] = next == proc ? NULL : next;
        prev->next_same_prio = next;
        next->prev_same_prio = prev;
        sys_state.n_runnable_procs --;
        sys_state.prio_proc_n[prio] --;
    }
    else if(proc->state != RUNNABLE && new_state == RUNNABLE){
        if(sys_state.prio_proc[prio] == NULL)
        {
            proc->next_same_prio = proc;
            proc->prev_same_prio = proc;
        }
        else {
            proc->next_same_prio = sys_state.prio_proc[prio];
            proc->prev_same_prio = sys_state.prio_proc[prio]->prev_same_prio;
            sys_state.prio_proc[prio]->prev_same_prio = proc;
            proc->prev_same_prio->next_same_prio = proc;
        }
        sys_state.prio_proc[prio] = proc;
        sys_state.n_runnable_procs ++;
        sys_state.prio_proc_n[prio] ++;
    }
    proc->state = new_state;
}

void add_child(proc_descriptor * parent, proc_descriptor * child){
    if(parent->child == NULL){
        child->next_sibling = child;
        child->prev_sibling = child;
    }
    else {
        child->next_sibling = parent->child;
        child->prev_sibling = parent->child->prev_sibling;
        parent->child->prev_sibling = child;
        child->prev_sibling->next_sibling = child;
    }
    parent->child = child;
    return;
}

void remove_child(proc_descriptor * parent, proc_descriptor * child){
    child->prev_sibling->next_sibling = child->next_sibling;
    child->next_sibling->prev_sibling = child->prev_sibling;
    if(parent->child == child)
        parent->child = child == child->next_sibling ? NULL : child->next_sibling;
    return;
}


/**** SYSCALLS ****/
void syscall_fork(){
    proc_descriptor * parent = &sys_state.procs[sys_state.last_pid];
    int child_pid = find_free_proc();
    if(child_pid == -1){
        /* No available slot */
        parent->err.no = MAX_PROC_REACHED;
        parent->saved_context.registers[0] = -1;
        return;
    }
    int priority = parent->saved_context.registers[0];
    if(priority > parent->priority){
        parent->err.no = TOO_HIGH_CHILD_PRIORITY;
        parent->saved_context.registers[0] = -1;
        return;
    }
    parent->saved_context.registers[0] = child_pid;
    proc_descriptor * child = &sys_state.procs[child_pid];
    make_child_proc_descriptor(child, parent, child_pid, priority);
    child->saved_context.registers[0] = 0;
    add_child(parent, child);
    change_state(child, RUNNABLE);
    /* Initializing MMU with copy and write */
    set_up_memory_new_proc(child);
    child->initialized = true;
    proc_verbose("Forked process %d into child %d of priority %d\r\n",
                 parent->pid, child->pid, child->priority);
    return;
}

void handle_zombie_child(proc_descriptor * parent, proc_descriptor * child){
    proc_verbose("Handling zombie child:\r\nParent : %d\r\nChild : %d\r\n",
        parent->pid, child->pid);
    uint64_t ret_status_addr = parent->saved_context.registers[0];
    parent->saved_context.registers[0] = child->pid;
    #ifdef PROC_VERBOSE
    print_err(child->err);
    #endif
    if(ret_status_addr != 0){
        parent->buffer.write_addr = ret_status_addr;
        assert(sizeof(err_t) <= BUFF_SIZE);
        memmove(parent->buffer.buff, &child->err, sizeof(err_t));
        parent->buffer.used_size = sizeof(err_t);
    }
    remove_child(parent, child);
    change_state(child, FREE);
    change_state(parent, RUNNABLE);
    return;
}

void syscall_wait(){
    proc_descriptor * parent = &sys_state.procs[sys_state.last_pid];
    proc_verbose("Syscall wait for process %d\r\n", parent->pid);
    if(parent->child == NULL){
        /* No child */
        proc_verbose("No child\r\n");
        parent->err.no = NO_CHILD;
        parent->saved_context.registers[0] = -1;
        return;
    }
    proc_descriptor * child = parent->child;
    /* here child != NULL */
    do
        child = child->next_sibling;
    while(child->state != ZOMBIE && child->next_sibling != parent->child);
    if(child->state == ZOMBIE)
        handle_zombie_child(parent, child);
    else
        /* No zombie child */
        change_state(parent, WAITING);
    return;
}

void syscall_exit(){
    proc_descriptor * proc = &sys_state.procs[sys_state.last_pid];
    proc_verbose("Syscall exit for process %d\r\n", proc->pid);
    proc->err.no   = proc->saved_context.registers[0];
    proc->err.data = proc->saved_context.registers[1];
    proc_descriptor * child = proc->child;
    if(child != NULL){
        do {
            child->parent_pid = 1;
            child = child->next_sibling;
        } while(child != proc->child);
    }
    proc_descriptor * parent = &sys_state.procs[proc->parent_pid];
    if(parent->state == WAITING)
        handle_zombie_child(parent, proc);
    else
        change_state(proc, ZOMBIE);
    return;
}

void handle_channel_connection(proc_descriptor * sender, proc_descriptor * receiver){
    proc_verbose("Handling channel connection:\r\nSender: %d\r\nReceiver: %d\r\n",
                 sender->pid, receiver->pid);
    /* we side with the receiver */
    if(receiver->receiver_data.receive_size < sender->sender_data.send_size){
        sender->err.no = SEND_DATA_TOO_LARGE;
        sender->saved_context.registers[0] = -1;
        return;
    }
    /* Needs acknowledge */
    sender->sender_data.acknowledged = false;
    /* If addr == null won't be written */
    assert(sender->buffer.used_size <= BUFF_SIZE);
    assert(sender->buffer.used_size <= receiver->receiver_data.receive_size);
    receiver->buffer.used_size = sender->buffer.used_size;
    receiver->buffer.write_addr = receiver->receiver_data.receive_data;
    memmove((void *) receiver->buffer.buff,
            (void *) sender->buffer.buff,
            sender->buffer.used_size);
    sender->buffer.used_size = 0;
    receiver->receiver_data.source_pid = sender->pid;
    receiver->saved_context.registers[0] = sender->pid;
    change_state(sender, SENDING_CH);
    change_state(receiver, RUNNABLE);
    return;
}

void syscall_send(){
    proc_descriptor * proc = &sys_state.procs[sys_state.last_pid];
    proc_verbose("Syscall send for process %d\r\n", proc->pid);
    int target_pid       =        proc->saved_context.registers[0];
    bool wait_for_listen = (bool) proc->saved_context.registers[5];
    size_t size          =        proc->saved_context.registers[2];
    if(target_pid < 0 || target_pid >= MAX_PROC){
        proc->err.no = INVALID_PID;
        proc->saved_context.registers[0] = -1;
        return;
    }
    else if(sys_state.procs[target_pid].state != LISTENING_CH
            && (!wait_for_listen)){
        proc->err.no = TARGET_NOT_LISTENING;
        proc->saved_context.registers[0] = -1;
        return;
    }
    else if(size > BUFF_SIZE){
        proc->err.no = SEND_DATA_TOO_LARGE;
        proc->saved_context.registers[0] = -1;
        return;
    }
    proc->sender_data.target_pid = proc->saved_context.registers[0];
    proc->sender_data.send_size  = proc->saved_context.registers[2];
    proc->sender_data.ack_data   = proc->saved_context.registers[3];
    proc->sender_data.ack_size   = proc->saved_context.registers[4];
    if(sys_state.procs[target_pid].state == LISTENING_CH)
        handle_channel_connection(proc, &sys_state.procs[target_pid]);
    else proc->state = WAIT_LISTENER;
    return;
}

void syscall_receive(){
    proc_descriptor * proc = &sys_state.procs[sys_state.last_pid];
    proc_verbose("Syscall receive for process %d\r\n", proc->pid);
    proc->receiver_data.receive_data = proc->saved_context.registers[0];
    proc->receiver_data.receive_size = proc->saved_context.registers[1];
    change_state(proc, LISTENING_CH);
    int i;
    for(i = 0; i < MAX_PROC; i++){
        if(sys_state.procs[i].state == WAIT_LISTENER &&
           sys_state.procs[i].sender_data.target_pid == proc->pid) break;
    }
    if(i < MAX_PROC)
        handle_channel_connection(&sys_state.procs[i], proc);
    return;
}

void handle_channel_ack(proc_descriptor * receiver, proc_descriptor * sender){
    proc_verbose("Handling channel ack:\r\nSender: %d\r\nReceiver: %d\r\n",
                 sender->pid, receiver->pid);
    if(sender->sender_data.acknowledged || sender->state != SENDING_CH){
        /* Error : nothing to acknowledge */
        receiver->err.no = SOURCE_ALREADY_ACKNOWLEDGED;
        receiver->saved_context.registers[0] = -1;
        return;
    }
    assert(sender->sender_data.target_pid == receiver->pid);
    /* Everything went well */
    receiver->saved_context.registers[0] = 0;
    /* we already have : */
    assert(receiver->buffer.used_size <= BUFF_SIZE);
    sender->buffer.used_size =
        sender->sender_data.ack_size < receiver->buffer.used_size ?
        sender->sender_data.ack_size : receiver->buffer.used_size;
    memmove((void *) sender->buffer.buff,
            (void *) receiver->buffer.buff,
            sender->buffer.used_size);
    sender->buffer.write_addr = sender->sender_data.ack_data;
    receiver->buffer.used_size = 0;
    sender->saved_context.registers[0] = receiver->receiver_data.return_code;
    change_state(sender, RUNNABLE);
    return;
}

void syscall_acknowledge(){
    proc_descriptor * proc = &sys_state.procs[sys_state.last_pid];
    proc_verbose("Syscall acknowledge for process %d\r\n", proc->pid);
    size_t size = proc->saved_context.registers[2];
    if(size > BUFF_SIZE){
        proc->err.no = SEND_DATA_TOO_LARGE;
        proc->saved_context.registers[0] = -1;
        return;
    }
    proc->receiver_data.return_code = proc->saved_context.registers[0];
    handle_channel_ack(proc, &sys_state.procs[proc->receiver_data.source_pid]);
    return;
}




/* Should not return */
__attribute__((__noreturn__))
void c_el1_svc_aarch64_handler(uint64_t esr_el1){
    #ifdef PROC_VERBOSE
    proc_verbose("C EL1 SVC AARCH64 Handler called\r\n");
    print_proc_descriptor(&sys_state.procs[sys_state.last_pid]);
    #endif
    uint16_t syscall = (esr_el1 & MASK(15,0));
    //get back syscall code (ARM ARM 2453 for encoding)
    switch(syscall){
    case 0:
        /* Fork */
        proc_verbose("Syscall code 0 : Fork\r\n");
        syscall_fork();
        break;
    case 1:
        /* Exit */
        proc_verbose("Syscall code 1 : Exit\r\n");
        syscall_exit();
        break;
    case 2:
        /* Wait */
        proc_verbose("Syscall code 2 : Wait\r\n");
        syscall_wait();
        break;
    case 3:
        /* Send */
        proc_verbose("Syscall code 3 : Send\r\n");
        syscall_send();
        break;
    case 4:
        /* Receive */
        proc_verbose("Syscall code 4 : Receive\r\n");
        syscall_receive();
        break;
    case 5:
        /* Acknowledge */
        proc_verbose("Syscall code 5 : Acknowledge\r\n");
        syscall_acknowledge();
        break;
    case 100:
        /* Halt syscall (halt cannot be executed at EL0) */
        proc_verbose("Syscall code 100 : Halt\r\n");
        halt();
        break;
    case 101:
        /* Test syscall : does nothing */
        proc_verbose("Syscall code 101 : Test id\r\n");
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
    schedule();
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

int get_parent_pid(int pid){
    return sys_state.procs[pid].parent_pid;
}

/****  SCHEDULER  ****/
__attribute__((__noreturn__))
void run_priority(int prio){
    proc_descriptor * proc = sys_state.prio_proc[prio];
    sys_state.prio_proc[prio] = proc->next_same_prio;
    if(proc->sched_conf.time_left == 0)
        proc->sched_conf.time_left = QUANTUM;
    proc_verbose("Running prio %d process %d\r\n", prio, proc->pid);
    run_process(proc);
}

__attribute__((__noreturn__))
void schedule(){
    proc_verbose("Entering schedule after execution of proc %d\r\n", sys_state.last_pid);
    #ifdef PROC_VERBOSE
    print_prio_lists();
    #endif
    if(sys_state.n_runnable_procs == 0){
        uart_info("No runnable processes\r\nHalting...\r\n");
        halt();
    }
    if(sys_state.prio_proc_n[MAX_PRIO] > 0)
        run_priority(MAX_PRIO);
    else if(sys_state.n_runnable_procs == sys_state.prio_proc_n[0])
        /* Only priorirty zero procs */
        run_priority(0);
    else{
        unsigned int proba[MAX_PRIO];
        proba[0] = 0;
        for(int i = 1; i < MAX_PRIO; i++){
            proba[i] =  sys_state.prio_proc_n[i] > 0 ? i * i + sys_state.prio_proc_n[i] : 0;
        }
        int prio = random_law(proba, MAX_PRIO);
        run_priority(prio);
    }
}
