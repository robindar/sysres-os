#include "mem_manager.h"
#include "../libk/uart.h"
#include "../memory/mmu.h"
#include "../libk/sys.h"

__attribute__((__noreturn__))
void main_mem_manager(){
    int pid, status;
    mem_request_t request = {0};
    while(1){
        pid = receive(&request, sizeof(mem_request_t));
        switch(request.code){
        case 1:
            #ifdef MALLOC_VERBOSE
            uart_verbose(
                "Memory manager processs reached over free request"
                " at 0x%x by %d\r\n", request.data, pid);
            #endif
            status = mem_manager_free_virtual_page(request.data, pid);
            status = acknowledge(status, NULL, 0);
            assert(status == 0);
        default:
            /* Unknown signal */
            #ifdef MALLOC_VERBOSE
            uart_error(
                "Memory manager processs reached over unknown request"
                "by %d\r\n", pid);
            #endif
            (void) acknowledge(-1, NULL, 0);
        }
    }
}
