.global c_serror_handler
.global c_irq_handler
.global c_fiq_handler
.global c_sync_handler
.section ".text.el3"
.globl Vector_table_el3
.balign 0x800
Vector_table_el3:
curr_el_sp0_sync:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x0, 3
        mov x1, 0
        bl el3_sync_handler

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
        mov x0, 3
        mov x1, 1
        bl el3_irq_handler

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
        mov x0, 3
        mov x1, 2
        bl el3_fiq_handler

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
        mov x0, 3
        mov x1, 3
        bl el3_serror_handler

.balign 0x80
.globl curr_el_spx_sync
curr_el_spx_sync:
        stp x0, x1,  [sp, #-16]!
        stp x2, x3,  [sp, #-16]!
        stp x4, x5,  [sp, #-16]!
        stp x6, x7,  [sp, #-16]!
        stp x8, x9,  [sp, #-16]!
        stp x10,x11, [sp, #-16]!
        stp x12,x13, [sp, #-16]!
        stp x14,x15, [sp, #-16]!
        mov x0, 3
        mov x1, 4
        bl el3_sync_handler
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
        mov x0, 3
        mov x1, 5
        bl el3_irq_handler

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
        mov x0, 3
        mov x1, 6
        bl el3_fiq_handler

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
        mov x0, 3
        mov x1, 7
        bl el3_serror_handler

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
        mov x0, 3
        mov x1, 8
        bl el3_sync_handler

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
        mov x0, 3
        mov x1, 9
        bl el3_irq_handler

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
        mov x0, 3
        mov x1, 10
        bl el3_fiq_handler

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
        mov x0, 3
        mov x1, 11
        bl el3_serror_handler

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
        mov x0, 3
        mov x1, 12
        bl el3_sync_handler

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
        mov x0, 3
        mov x1, 13
        bl el3_irq_handler

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
        mov x0, 3
        mov x1, 14
        bl el3_fiq_handler

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
        mov x0, 3
        mov x1, 5
        bl el3_serror_handler

el3_sync_handler:
        mrs x2, spsr_el3
        mrs x3, elr_el3
        stp x2, x3, [sp, #-16]!
        mrs x4, esr_el3
        mrs x5, far_el3
        bl c_sync_handler
        ldp x2, x3, [sp], #16
        msr spsr_el3, x2
        msr elr_el3, x3
        ldp x14,x15, [sp], #16
        ldp x12,x13, [sp], #16
        ldp x10,x11, [sp], #16
        ldp x8, x9,  [sp], #16
        ldp x6, x7,  [sp], #16
        ldp x4, x5,  [sp], #16
        ldp x2, x3,  [sp], #16
        ldp x0, x1,  [sp], #16
        eret

el3_serror_handler:
        mrs x2, spsr_el3
        mrs x3, elr_el3
        stp x2, x3, [sp, #-16]!
        mrs x4, esr_el3
        mrs x5, far_el3
        bl c_serror_handler
        ldp x2, x3, [sp], #16
        msr spsr_el3, x2
        msr elr_el3, x3
        ldp x14,x15, [sp], #16
        ldp x12,x13, [sp], #16
        ldp x10,x11, [sp], #16
        ldp x8, x9,  [sp], #16
        ldp x6, x7,  [sp], #16
        ldp x4, x5,  [sp], #16
        ldp x2, x3,  [sp], #16
        ldp x0, x1,  [sp], #16
        eret


el3_irq_handler:
        mrs x2, spsr_el3
        mrs x3, elr_el3
        stp x2, x3, [sp, #-16]!
        bl c_irq_handler
        ldp x2, x3, [sp], #16
        msr spsr_el3, x2
        msr elr_el3, x3
        ldp x14,x15, [sp], #16
        ldp x12,x13, [sp], #16
        ldp x10,x11, [sp], #16
        ldp x8, x9,  [sp], #16
        ldp x6, x7,  [sp], #16
        ldp x4, x5,  [sp], #16
        ldp x2, x3,  [sp], #16
        ldp x0, x1,  [sp], #16
        eret

el3_fiq_handler:
        mrs x2, spsr_el3
        mrs x3, elr_el3
        stp x2, x3, [sp, #-16]!
        bl c_fiq_handler
        ldp x2, x3, [sp], #16
        msr spsr_el3, x2
        msr elr_el3, x3
        ldp x14,x15, [sp], #16
        ldp x12,x13, [sp], #16
        ldp x10,x11, [sp], #16
        ldp x8, x9,  [sp], #16
        ldp x6, x7,  [sp], #16
        ldp x4, x5,  [sp], #16
        ldp x2, x3,  [sp], #16
        ldp x0, x1,  [sp], #16
        eret
