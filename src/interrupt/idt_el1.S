.global c_serror_handler
.global c_irq_handler
.global c_fiq_handler
.global c_sync_handler
.global c_el1_svc_aarch64_handler
.section ".text.el1"
.globl Vector_table_el1
.balign 0x800
Vector_table_el1:
curr_el_sp0_sync:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x8, 0
    //I changed from bl to b to preserve x30
    b el1_sync_handler

.balign 0x80
curr_el_sp0_irq:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x0, 1
    mov x1, 1
    b el1_irq_handler

.balign 0x80
curr_el_sp0_fiq:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x0, 1
    mov x1, 2
    b el1_fiq_handler

.balign 0x80
curr_el_sp0_serror:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x0, 1
    mov x1, 3
    b el1_serror_handler

.balign 0x80
curr_el_spx_sync:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x8, 4
    b el1_sync_handler

.balign 0x80
curr_el_spx_irq:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x0, 1
    mov x1, 5
    b el1_irq_handler

.balign 0x80
curr_el_spx_fiq:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x0, 1
    mov x1, 6
    b el1_fiq_handler

.balign 0x80
curr_el_spx_serror:
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mov x0, 1
    mov x1, 7
    b el1_serror_handler

 .balign 0x80
lower_el_aarch64_sync:
    //Syscall handler
    b lower_el_el1_sync_handler

.balign 0x80
lower_el_aarch64_irq:
    mov x0, 1
    mov x1, 9
    b el1_irq_handler

.balign 0x80
lower_el_aarch64_fiq:
    mov x0, 1
    mov x1, 10
    b el1_fiq_handler

.balign 0x80
lower_el_aarch64_serror:
    mov x0, 1
    mov x1, 11
    b el1_serror_handler

.balign 0x80
lower_el_aarch32_sync:
    b lower_el_el1_sync_handler

.balign 0x80
lower_el_aarch32_irq:
    mov x0, 1
    mov x1, 13
    b el1_irq_handler

.balign 0x80
lower_el_aarch32_fiq:
    mov x0, 1
    mov x1, 14
    b el1_fiq_handler

.balign 0x80
lower_el_aarch32_serror:
    mov x0, 1
    mov x1, 5
    b el1_serror_handler

    // This a sync interrupt coming from EL0, it may be a syscall.
    // If it's not we follwo the std path
    // (Tranlsation faults at EL0 will be handled in memory/mmu.c)
    //Here we expect nothing to be done
lower_el_el1_sync_handler:
    msr spsel, xzr                  //Switch to SP_EL0 (not to destroy application stack)
    stp x0, x1,  [sp, #-16]!
    stp x2, x3,  [sp, #-16]!
    stp x4, x5,  [sp, #-16]!
    stp x6, x7,  [sp, #-16]!
    stp x8, x9,  [sp, #-16]!
    stp x10,x11, [sp, #-16]!
    stp x12,x13, [sp, #-16]!
    stp x14,x15, [sp, #-16]!
    mrs x4, esr_el1
    //Now we want to read exception class to determine
    //if it's a syscall :
    //if it is one, we want it to be fast
    and x11, x4, #0xfc000000        //Mask to get Exception Class
    lsr x11, x11, 26                //21 -> SVC AArch64
    cmp x11, #21
    bne el1_sync_handler            //If SVC call, don-t jump
    //It-s a SVC call
    //Now backup everything : there will probably have a context switch
    //Don't forget to clean this
    stp x16, x17,  [sp, #-16]!
    stp x18, x19,  [sp, #-16]!
    stp x20, x21,  [sp, #-16]!
    stp x22, x23,  [sp, #-16]!
    stp x24, x25,  [sp, #-16]!
    stp x26, x27,  [sp, #-16]!
    stp x28, x29,  [sp, #-16]!
    stp x30, x30,  [sp, #-16]!      //Stack has to be 16-byte aligned
    add x0, sp, #(16 * 16)          //Backup SP_EL0 (as it was before interrupt)
    mrs x1, elr_el1
    mrs x2, spsr_el1                //Backup of pstate
    mov x3, sp                      //Handler SP
    bl save_context
    add sp, sp, #(32 * 8)           //Clean stack
    mrs x0, esr_el1                 //Move ESR_EL1 to X0
    mov x2, #1
    msr spsel, x2                   //Switch back to SP_EL1
    ldr x2, = __mmu_tables_start ;  //Switch to kernel MMU to avoid breaking anything for the process
    msr ttbr0_el1, x2               // Set ttbr0
    b c_el1_svc_aarch64_handler     //Warning this musn't return here

    //Standard sync handler (no svc) : we expect x0-x15 to be saved on the stack
    //If GCC follows the calling conventions, we only need to backup x29-x30 in addition to x0-x15
    //Below n denotes the EL from which the exception is coming
    //Either we are coming from lower_el_el1_sync_handler and so x0-x15 are back up with SP_EL0
    //Otherwise, we are coming from either an EL1 exception and we save everything on SP_EL1
    //Or we are coming from an unhandled exception source and the whole system crashes, so we don't save anything
el1_sync_handler:
    //For now everything is aved with SP_ELn
    stp x29,x30, [sp, #-16]!        //Save x29, x30
    mrs x9, spsr_el1                //Backup spsr_el1, elr_el1 as explained in the doc
    mrs x10, elr_el1
    stp x9, x10, [sp, #-16]!
    mrs x9, SPSel                   //Backup SPSel ie n
    mrs x10, TTBR0_EL1              //Backup TTBR0_EL1 (easier for restore)
    mov x0, #1
    msr SPSel, x0                   //Make sure we are with SP_EL1
    stp x9, x10, [sp, #-16]!
    ldr x10, = __mmu_tables_start ;  //Switch to kernel MMU to avoid breaking anything for the process
    msr TTBR0_EL1, x10
    ISB
    mov x0, 1
    mov x1, x8
    mrs x2, spsr_el1
    mrs x3, elr_el1
    mrs x4, esr_el1
    mrs x5, far_el1
    bl c_sync_handler
    ldp x9, x10, [sp], #16
    msr TTBR0_EL1, x10
    ISB
    msr SPSel, x9                 //We are back with SP_ELn where we left our things
    ldp x2, x3, [sp], #16
    msr spsr_el1, x2
    msr elr_el1, x3
    ldp x29,x30, [sp], #16
    ldp x14,x15, [sp], #16
    ldp x12,x13, [sp], #16
    ldp x10,x11, [sp], #16
    ldp x8, x9,  [sp], #16
    ldp x6, x7,  [sp], #16
    ldp x4, x5,  [sp], #16
    ldp x2, x3,  [sp], #16
    ldp x0, x1,  [sp], #16
    eret

el1_serror_handler:
    stp x29,x30, [sp, #-16]!//Save x29, x30
    mrs x2, spsr_el1
    mrs x3, elr_el1
    stp x2, x3, [sp, #-16]!
    mrs x4, esr_el1
    mrs x5, far_el1
    bl c_serror_handler
    ldp x2, x3, [sp], #16
    msr spsr_el1, x2
    msr elr_el1, x3
    ldp x29,x30, [sp], #16
    ldp x14,x15, [sp], #16
    ldp x12,x13, [sp], #16
    ldp x10,x11, [sp], #16
    ldp x8, x9,  [sp], #16
    ldp x6, x7,  [sp], #16
    ldp x4, x5,  [sp], #16
    ldp x2, x3,  [sp], #16
    ldp x0, x1,  [sp], #16
    eret


el1_irq_handler:
    stp x29,x30, [sp, #-16]!//Save x29, x30
    mrs x2, spsr_el1
    mrs x3, elr_el1
    stp x2, x3, [sp, #-16]!
    bl c_irq_handler
    ldp x2, x3, [sp], #16
    msr spsr_el1, x2
    msr elr_el1, x3
    ldp x29,x30, [sp], #16
    ldp x14,x15, [sp], #16
    ldp x12,x13, [sp], #16
    ldp x10,x11, [sp], #16
    ldp x8, x9,  [sp], #16
    ldp x6, x7,  [sp], #16
    ldp x4, x5,  [sp], #16
    ldp x2, x3,  [sp], #16
    ldp x0, x1,  [sp], #16
    eret

el1_fiq_handler:
    stp x29,x30, [sp, #-16]!//Save x29, x30
    mrs x2, spsr_el1
    mrs x3, elr_el1
    stp x2, x3, [sp, #-16]!
    bl c_fiq_handler
    ldp x2, x3, [sp], #16
    msr spsr_el1, x2
    msr elr_el1, x3
    ldp x29,x30, [sp], #16
    ldp x14,x15, [sp], #16
    ldp x12,x13, [sp], #16
    ldp x10,x11, [sp], #16
    ldp x8, x9,  [sp], #16
    ldp x6, x7,  [sp], #16
    ldp x4, x5,  [sp], #16
    ldp x2, x3,  [sp], #16
    ldp x0, x1,  [sp], #16
    eret
