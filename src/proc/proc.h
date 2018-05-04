#ifndef PROC_H
#define PROC_H

#include <stdbool.h>
#include <stdint.h>


/* We will follow first the specifications of the micro-kernel given during the classs */

/* we have 16 bits for ASID according to memory/init_mmu.s */
/* but for now we will only allow 256 proc */
/* see doc/mmu.md for why only 32 */
#define MAX_PROC  32
#define N_REG     31            /* x0-x30 */

enum proc_state {
    FREE = 0,                   /* Unused proc slot */
    RUNNABLE,                   /* Ready to be executed */
    WAITING,                    /* for one of its child to die, after a wait call */
    ZOMBIE,                     /* After Exit */
    READING,                    /* Blocked Reading channel */
    WRITING,                    /* Blocked Writing channel */
    KERNEL,                     /* Process 0 is Kernel (convention) */
};

enum sched_policy {
    DEFAULT                     /* TODO : add realtime non preemptible, realtime preemptible, normal like Linux ? */
};

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

    /* Indicates whether its address space has been initialized*/
    /* ie whether start_process has ever been called on it */
    bool initialized;
} mem_conf;

typedef struct{
    int pid;
    int parent_pid;
    int priority; /* Priority from 0 to 15, 15 being the highest */
    enum proc_state state;
    enum sched_policy sched_policy;
    context saved_context;
    mem_conf mem_conf;
    /* TODO : channels */
} proc_descriptor;

typedef struct{
    int curr_pid;
    proc_descriptor procs[MAX_PROC];
} system_state;

void init_proc();
int exec_proc(int pid);
#endif
