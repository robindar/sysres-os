.global c_el2_handler
.section ".text.el2"
.globl Vector_table_el2
.balign 0x800
Vector_table_el2:
curr_el_sp0_sync:
        b c_el2_handler

.balign 0x80
curr_el_sp0_irq:
        b c_el2_handler

.balign 0x80
curr_el_sp0_fiq:
        b c_el2_handler

.balign 0x80
curr_el_sp0_serror:
        b c_el2_handler

.balign 0x80
curr_el_spx_sync:
        b c_el2_handler

.balign 0x80
curr_el_spx_irq:
        b c_el2_handler

.balign 0x80
curr_el_spx_fiq:
        b c_el2_handler

.balign 0x80
curr_el_spx_serror:
        b c_el2_handler

 .balign 0x80
lower_el_aarch64_sync:
        b c_el2_handler

.balign 0x80
lower_el_aarch64_irq:
        b c_el2_handler

.balign 0x80
lower_el_aarch64_fiq:
        b c_el2_handler

.balign 0x80
lower_el_aarch64_serror:
        b c_el2_handler

.balign 0x80
lower_el_aarch32_sync:
        b c_el2_handler

.balign 0x80
lower_el_aarch32_irq:
        b c_el2_handler

.balign 0x80
lower_el_aarch32_fiq:
        b c_el2_handler

.balign 0x80
lower_el_aarch32_serror:
        b c_el2_handler
