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
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x8, 8
        b el1_sync_handler

.balign 0x80
lower_el_aarch64_irq:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x0, 1
        mov x1, 9
        b el1_irq_handler

.balign 0x80
lower_el_aarch64_fiq:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x0, 1
        mov x1, 10
        b el1_fiq_handler

.balign 0x80
lower_el_aarch64_serror:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x0, 1
        mov x1, 11
        b el1_serror_handler

.balign 0x80
lower_el_aarch32_sync:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x8, 12
        b el1_sync_handler

.balign 0x80
lower_el_aarch32_irq:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x0, 1
        mov x1, 13
        b el1_irq_handler

.balign 0x80
lower_el_aarch32_fiq:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x0, 1
        mov x1, 14
        b el1_fiq_handler

.balign 0x80
lower_el_aarch32_serror:
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
        b el1_serror_handler

el1_sync_handler:
        stp x29,x30, [sp, #-16]!//Save x29, x30
        mrs x9, spsr_el1        //Backup spsr_el1, elr_el1 as explained in the doc
        mrs x10, elr_el1
        stp x9, x10, [sp, #-16]!
        mrs x11, esr_el1
        //Now we want to read exception class to determine
        //if it's a syscall :
        //if it is one, we want it to be fast
        and x11, x11, #0xfc000000 //Mask to get Exception Class
        lsr x11, x11, 26          //21 -> SVC AArch64
        cmp x11, #21
        beq c_el1_svc_aarch64_handler //If SVC call, jump
        mov x0, 1                  //Otherwise, put eveyrthing for info for debug
        mov x1, x8
        mov x2, x9
        mov x3, x10
        mrs x4, esr_el1
        mrs x5, far_el1
        bl c_sync_handler
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
