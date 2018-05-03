#include "test.h"
#include "../memory/mmu.h"

/* (Warning this requires a special case in the translation fault handler
   to restore the pages belonging to id_paging
   moreover make sure there are no uart_print during the handling of translation fault error)*/
/* If this is not the case, the shouldn't happen shouldn't be printed (the system will loop)*/


void free_test(){
    uint64_t phys_addr = 0x7000;
    free_virtual_page(phys_addr);
    asm volatile("ISB");
    uint64_t data = *((uint64_t*)phys_addr);
    uart_debug("Shouldn't happen\r\n");
    uart_debug("hey = %x\r\n", data);
}
