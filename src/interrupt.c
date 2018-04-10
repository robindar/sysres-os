#include "interrupt.h"

void display_esr_eln_info(uint64_t esr_eln){
    //Parse ESR_EL1 (see aarch64, exception and interrupt handling)
    uint64_t exception_class = (esr_eln & 0xfc000000) >> 26;
    bool il = (esr_eln & 0x2000000) >> 25;
    uint16_t instr_specific_syndrom = (esr_eln & 0x1fffff);
    uart_info(
        "ER_ELn info :\r\nException Class : 0b%b\r\nIL : %d\r\n"
         "Instruction Specific Syndrom : 0x%x\r\n",
        exception_class, il, instr_specific_syndrom);
}

void display_pstate_info(uint64_t pstate){
    bool n  = (pstate & (1 << 31)) >> 31;
    bool z  = (pstate & (1 << 30)) >> 30;
    bool c  = (pstate & (1 << 29)) >> 29;
    bool v  = (pstate & (1 << 28)) >> 28;
    bool ua = (pstate & (1 << 23)) >> 23;
    bool ss = (pstate & (1 << 21)) >> 21;
    bool il = (pstate & (1 << 20)) >> 20;
    bool d  = (pstate & (1 <<  9)) >>  9;
    bool a  = (pstate & (1 <<  8)) >>  8;
    bool i  = (pstate & (1 <<  7)) >>  7;
    bool f  = (pstate & (1 <<  6)) >>  6;
    bool ar = (pstate & (1 <<  5)) >>  5;
    int  m  = (pstate & (3 <<  2)) >>  2;
    bool sp = (pstate & 1);
    uart_info(
        "PSTATE info :\r\n"
        "Negative condition flag : %d\r\n"
        "Zero     condition flag : %d\r\n"
        "Carry    condition flag : %d\r\n"
        "Overflow condition flag : %d\r\n"
        "Debug mask              : %d\r\n"
        "System Error mask       : %d\r\n"
        "IRQ mask                : %d\r\n"
        "FIQ mask                : %d\r\n"
        "Software step           : %d\r\n"
        "UA0                     : %d\r\n"
        "Illegal execution state : %d\r\n"
        "Architecture            : %d\r\n"
        "Mode field              : EL%d\r\n"
        "Stack Pointer           : %d\r\n"
                , n,z,c,v,d,a,i,f,ss,ua,il,ar,m,sp);
}

void display_error(char * msg, uint64_t el, uint64_t nb, uint64_t spsr_el, uint64_t elr_el, uint64_t esr_el, uint64_t far_el){
    uart_error(
        "%s :\r\nAt level : EL%d\r\nCase nb :%d\r\n"
        "ELR_EL : 0x%x\r\nSPSR_EL : 0x%x\r\nESR_EL : 0x%x\r\nFAR_EL : 0x%x\r\n",
        msg,el,nb, elr_el, spsr_el, esr_el, far_el);
    display_esr_eln_info(esr_el);
    display_pstate_info(elr_el);
    abort();
}

void instruction_abort_handler(uint64_t el, uint64_t nb, uint64_t spsr_el, uint64_t elr_el, uint64_t esr_el, uint64_t far_el, bool lower_el){
        uint64_t instruction_fault_status_code = esr_el & MASK(5,0);
        switch(instruction_fault_status_code){
        case 0b110:             /* Transltation fault level 2 */
            translation_fault_handler(far_el, 2, lower_el);
            break;
        case 0b111:             /* Transltation fault level 3 */
            translation_fault_handler(far_el, 3, lower_el);
            break;
        default:
            display_error("Instruction Abort Error", el, nb, spsr_el, elr_el, esr_el, far_el);
        }
}
void data_abort_handler(uint64_t el, uint64_t nb, uint64_t spsr_el, uint64_t elr_el, uint64_t esr_el, uint64_t far_el, bool lower_el){
        uint64_t data_fault_status_code = esr_el & MASK(5,0);
        switch(data_fault_status_code){
        case 0b110:             /* Transltation fault level 2 */
            translation_fault_handler(far_el, 2, lower_el);
            break;
        case 0b111:             /* Transltation fault level 3 */
            translation_fault_handler(far_el, 3, lower_el);
            break;
        default:
            display_error("Data Abort Error", el, nb, spsr_el, elr_el, esr_el, far_el);
        }
}

void translation_fault_handler(uint64_t fault_address, int level, bool lower_lvl){};

void c_sync_handler(uint64_t el, uint64_t nb, uint64_t spsr_el, uint64_t elr_el, uint64_t esr_el, uint64_t far_el){
    /* el indicates exception level */
    uint64_t exception_class = (esr_el & 0xfc000000) >> 26;
    /* See ARm ARM 1878 for EC constants */
    switch(exception_class){
    case 0b100000:          /* Instruction Abort from a lower Exception level */
        instruction_abort_handler(el, nb, spsr_el, elr_el, esr_el, far_el, true);
        break;
    case 0b100001:          /* Instruction Abort without a change in Exception level */
        instruction_abort_handler(el, nb, spsr_el, elr_el, esr_el, far_el, false);
        break;
    case 0b100100:          /* Data Abort from a lower Exception level */
        data_abort_handler(el, nb, spsr_el, elr_el, esr_el, far_el, true);
        break;
    case 0b100101:          /* Data Abort without a change in Exception level */
        data_abort_handler(el, nb, spsr_el, elr_el, esr_el, far_el, false);
        break;
    default:
        display_error("Sync Exception Error", el, nb, spsr_el, elr_el, esr_el, far_el);
    }

}


void c_serror_handler(uint64_t el, uint64_t nb, uint64_t elr_el, uint64_t spsr_el, uint64_t esr_el, uint64_t far_el){
    uart_error(
        " System Error :\r\nAt level : EL%d\r\nCase nb :%d\r\n"
        "ELR_EL = 0x%x\r\nSPSR_EL = 0x%x\r\nESR_EL = 0x%x\r\nFAR_EL = 0x%x\r\n",
        el, nb, elr_el, spsr_el, esr_el, far_el);
    display_esr_eln_info(esr_el);
    display_pstate_info(elr_el);
    abort();
}

/* TODO : get back info on the interrupt from GIC */
void c_irq_handler(uint64_t el, uint64_t nb){
    uart_error(
        " IRQ :\r\nAt level : EL%d\r\nCase nb :%d\r\n",
        el, nb);
    abort();
}

void c_fiq_handler(uint64_t el, uint64_t nb){
    uart_error(
        " FIQ :\r\nAt level : EL%d\r\nCase nb :%d\r\n",
        el,nb);
    abort();
}

void c_el2_handler(){
    uart_error("Interrupt handled at EL2 : non supported\r\nAborting...\r\n");
    abort();
}

void c_el1_svc_aarch64_handler(uint64_t x0,uint64_t x1,uint64_t x2,uint64_t x3,
                               uint64_t x4,uint64_t x5,uint64_t x6,uint64_t x7){
    //Syscall arguments
    (void) x0;
    (void) x1;
    (void) x2;
    (void) x3;
    (void) x4;
    (void) x5;
    (void) x6;
    (void) x7;
    uint64_t esr_el1;
    asm ("mrs %0, esr_el1":"=r"(esr_el1)::);
    uint16_t syscall = (esr_el1 & 0x1ffffff); //get back syscall code
    switch(syscall){
        default:
            uart_error("Error no syscall SVC Aarch64 corresponding to code %d\r\n", syscall);
            display_esr_eln_info(esr_el1);
            uart_error("Aborting...\r\n");
            abort();
    }
}
