.global c_el2_handler
.section ".text.el2"
.globl Vector_table_el2
.balign 0x800
Vector_table_el2:
curr_el_sp0_sync:
        bl c_el2_handler

.balign 0x80
curr_el_sp0_irq:
        bl c_el2_handler

.balign 0x80
curr_el_sp0_fiq:
        bl c_el2_handler

.balign 0x80
curr_el_sp0_serror:
        bl c_el2_handler

.balign 0x80
curr_el_spx_sync:
        bl c_el2_handler

.balign 0x80
curr_el_spx_irq:
        bl c_el2_handler

.balign 0x80
curr_el_spx_fiq:
        bl c_el2_handler

.balign 0x80
curr_el_spx_serror:
        bl c_el2_handler

 .balign 0x80
lower_el_aarch64_sync:
        bl c_el2_handler

.balign 0x80
lower_el_aarch64_irq:
        bl c_el2_handler

.balign 0x80
lower_el_aarch64_fiq:
        bl c_el2_handler

.balign 0x80
lower_el_aarch64_serror:
        bl c_el2_handler

.balign 0x80
lower_el_aarch32_sync:
        bl c_el2_handler

.balign 0x80
lower_el_aarch32_irq:
        bl c_el2_handler

.balign 0x80
lower_el_aarch32_fiq:
        bl c_el2_handler

.balign 0x80
lower_el_aarch32_serror:
        bl c_el2_handler
