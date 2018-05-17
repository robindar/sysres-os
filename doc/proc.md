## Processus ##

We will follow the specs of the micro-kernel given during the class (at least in a firs time)

# Syscalls #

Syscalls will have to be directed at EL1 through a SVC call. The SVC argument will determine the type of the Sysacll, and not r0 as specified in the spec.
They can pass any number of arguments in the registers

Note : WFE cannot be executed at EL0 (if it leads to low power mode)
Thus Syscall : 100 -> halt

# MMU #
The Address Space ID (ASID) is defined as the PID of the process.
For the kernel, it is 0 by convention.
Init will have pid 1 and thus ASID 1.
TCR_EL1[22] is set to 0 thus for now it the ASID of TTBR0_EL1 that prevails (and not the one from TTBR1_EL1). Moreover TCR_EL1[34] is set to 1 (see memory/init_mmu.s) thus the ASID can take at most 16 bits (see ARM ARM 2691-2692).

For now : Memory plan:
First kernel
Then Stack/Heap
(as the code is actually in the kernel for now)
=> Hence the kernel can be set as global


# Things to save and restore #
- TTBR0_EL1 :
            - Doesn't need to be back up at process interruption as it is fixed for each proc and determined at the first run (same for kernel)
            - However, the register and the global var need to be updated :
            - Run process :
              Change TTBR0_EL1 and do a call to set_lvl2_table_address_from_TTBR0_EL1 (AT EL1 ! TTBR0_EL1 cannot be accessed at EL0) (see swict_to_proc in proc_mmu.c)
            - Process interruption :
              Restore kernel TTBR0_EL1 (done in lower_el_el1_sync_handler AFTER a call to backup context) and call set_lvl2_table_address_from_TTBR0_EL1
              (in c_el1_svc_aarch64_sync_handler)
            - Thus there is no need for set_lvl2_table_address_from_TTBR0_EL1 to be called in mmu.c


# Procs reserved #
- 0 : Kernel
- 1 : Init

# Process return conventions
  Processes may return a err_t instead of simple int

# Syscalls #



- 0 Fork :
  int fork(int priority);
  - Arg : x0 : priority of the child
  - Return value in x0:
    - Child pid on sucess
    - -1 on failure and errno set accordingly
- 1 Exit :
  void exit(errno_t no, errdata_t data);
- 2 Wait :
  int wait(err_t * status);
  - Arg : x0 : pointer to a err_t sruct to store proc return data if non NULL
  - Return value : x0 :
    - Child pid on success
    - -1 on failure and errno set accordingly



# Timer #
Due to material limitations, ie if IRQ are masked in PSTATE (after an exc at EL1 for ex) we have no way of knowing that a timer has timed out, we do the follwoing :
- On a sync exception from EL0 != SVC(the only one which can resume execution) we disable IRQ in timer control. this way is_timer_finished reports accuratly the end of the timer.
  We only have, before returning to EL0, to restart the timer with a very small time (EPSILON) if the timer had timed out or with its curr value o/w
- On a syscall we switch off the timer and back up its value.

The Epsilon plays a special role :
- It is added to the timer whenever we are not sure it will make it through the switch
  Ex : translation_fault -> before resuming exec, timer < epsilon : we put it at epsilon so it can go through the switch
