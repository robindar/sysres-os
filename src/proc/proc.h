#ifndef PROC_H
#define PROC_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "../libk/errno.h"
#include "../interrupt/timer.h"

/* We will follow first the specifications of the micro-kernel given during the classs */

/* we have 16 bits for ASID according to memory/init_mmu.s */
/* but for now we will only allow 256 proc */
/* see doc/mmu.md for why only 32 */
#define MAX_PROC   32
#define N_REG      31            /* x0-x30 */
#define BUFF_SIZE 512
#define MAX_PRIO   15

enum proc_state {
    FREE = 0,                   /* Unused proc slot */
    RUNNABLE,                   /* Ready to be executed */
    WAITING,                    /* for one of its child to die, after a wait call */
    ZOMBIE,                     /* After Exit */
    SENDING_CH,                 /* Blocked Sending on channel */
    LISTENING_CH,               /* Blocked Listning on channel */
    WAIT_LISTENER,              /* Blocked waiting listener */
    KERNEL,                     /* Kernel */
};

typedef struct {
    uint32_t time_left;
    bool preempt;
} sched_conf;

typedef struct {
    uint64_t registers[N_REG];
    uint64_t pc;                /* This is the address where we have to start again*/
                                /* It is provided by ELR after a syscall*/
    uint64_t sp;
    uint64_t pstate;
} context;

typedef struct {
    uint64_t ttbr0_el1;         /* Contains ASID = PID and lvl2_base_address */
    /* -> this is a bit redundant with our new scheme for mmu table alloc but we'll keep it anyway */
    /* For now we don't use ttbr1_el1*/

    /* Backup of alloc.c global var */
    uint64_t heap_begin;
    int end_offset;
    void * global_base;
} mem_conf;

typedef struct {
    int target_pid;
    size_t send_size;
    uint64_t ack_data;
    size_t ack_size;
    bool acknowledged;
} sender_data;

typedef struct {
    int source_pid;
    size_t receive_size;
    uint64_t receive_data;
    int return_code;
} receiver_data;

typedef struct {
    char buff[BUFF_SIZE];
    size_t used_size;           /* in bytes */
    uint64_t write_addr;        /* if !=0, buff is written here */
} buffer;

typedef struct proc_descriptor {
    int pid;
    int parent_pid;
    int priority; /* Priority from 0 to 15, 15 being the highest */
    enum proc_state state;
    sched_conf sched_conf;
    /* Indicates whether this process has already been run/ initialized */
    bool initialized;
    context saved_context;
    mem_conf mem_conf;
    err_t err;
    /* FILO struct, NULL when empty */
    /* non cyclical simply linked list */
    struct proc_descriptor * child;
    struct proc_descriptor * next_sibling;
    struct proc_descriptor * prev_sibling;

    buffer buffer;
    sender_data sender_data;
    receiver_data receiver_data;

    /* cyclical simply linked list */
    struct proc_descriptor * next_same_prio;
    struct proc_descriptor * prev_same_prio;
} proc_descriptor;

typedef struct{
    int curr_pid;
    int last_pid;               /* Only used if curr_pid == 0 */
    proc_descriptor procs[MAX_PROC];
    unsigned int prio_proc_n[MAX_PRIO + 1]; /* nb of runnable procs in a given prio*/
    unsigned int n_runnable_procs;
    proc_descriptor * prio_proc[MAX_PRIO + 1];
} system_state;

void init_proc();
int exec_proc(int pid);
void restore_errno(const proc_descriptor * proc);
/* WARNING : kernel memory alloc functions mustn't be called after this*/
void restore_alloc_conf(const proc_descriptor * proc);
void save_alloc_conf(proc_descriptor * proc);
uint64_t get_lvl2_address_from_sys_state(int pid);
int get_curr_pid();
int get_parent_pid(int pid);
void change_state(proc_descriptor * proc, enum proc_state new_state);

__attribute__((__noreturn__))
void schedule();
#endif
