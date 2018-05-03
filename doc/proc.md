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



