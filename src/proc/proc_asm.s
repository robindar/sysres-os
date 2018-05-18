.globl restore_and_run
restore_and_run:
    //We have :
    //x0: &registers[30]
    //x1: pc
    //x2: sp
    //x3: pstate
    //x4: TTBR0_EL1
    msr TTBR0_EL1, x4                 //Switch to proc MMU
    ISB                               //Done here bc we mustn't touch EL1 stack after
    msr spsr_el1, x3
    msr  elr_el1, x1
    msr spsel, xzr                     //Switch to SP_EL0 stack pointer
    mov sp, x2                         //Restore SP_EL0
    //here we restore buff
    stp x0, x1, [sp, #-16]!             //Backup x0
    bl write_buff_svc
    ldp x0, x1, [sp], #16
    mov x2, #1
    msr spsel, x2                      //Switch back to SP_EL1
    ldr x2, =0x3F200000                //Clean EL1 stack
    mov sp, x2
    ldr x30,     [x0], #(-16)           //Post-incr
    ldp x28,x29, [x0], #(-16)
    ldp x26,x27, [x0], #(-16)
    ldp x24,x25, [x0], #(-16)
    ldp x22,x23, [x0], #(-16)
    ldp x20,x21, [x0], #(-16)
    ldp x18,x19, [x0], #(-16)
    ldp x16,x17, [x0], #(-16)
    ldp x14,x15, [x0], #(-16)
    ldp x12,x13, [x0], #(-16)
    ldp x10,x11, [x0], #(-16)
    ldp x8, x9,  [x0], #(-16)
    ldp x6, x7,  [x0], #(-16)
    ldp x4, x5,  [x0], #(-16)
    ldp x2, x3,  [x0], #(-16)
    ldp x0, x1,  [x0]        //No write-back here (thx AS for the warning)
    eret
