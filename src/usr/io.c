#include "../libk/uart.h"
#include "../libk/sys.h"
#include "io.h"
#include "../libk/debug.h"

#define IO_VERBOSE
#ifdef IO_VERBOSE
#define io_verbose(...) uart_verbose(__VA_ARGS__)
#else
#define io_verbose(...) ((void) 0)
#endif

__attribute__((__noreturn__))
void main_io_manager(){
    assert(get_curr_pid() == IO_MANAGER_PID);
    int pid, status = 0;
    io_request_t   request = {0};
    io_response_t response = {0};
    while(1){
        pid = receive(&request, sizeof(io_request_t));
        switch(request.code){
        case 0:
            /* putc */
            io_verbose("IO_Manager: Putc\r\n");
            uart_putc((unsigned char) request.data);
            status = acknowledge(0, NULL, 0);
            break;
        case 1:
            /* put_int */
            io_verbose("IO_Manager: Put_int\r\n");
            status = uart_put_int((int64_t) request.data,
                                  (unsigned int) request.buff[0],
                                  (bool)         request.buff[1],
                                  (bool)         request.buff[2]);
            status = acknowledge(status, NULL, 0);
            break;
        case 2:
            /* puts */
            io_verbose("IO_Manager: Puts\r\n");
            status = uart_puts(request.buff);
            status = acknowledge(status, NULL, 0);
            break;
        case 3:
            /* get_string */
            io_verbose("IO_Manager: Get_string\r\n");
            status = uart_get_string(response.buff, IO_BUFF_SIZE);
            status = acknowledge(status, &response, sizeof(io_response_t));
            break;
        default:
            /* Unknown signal */
            uart_error(
                "IO processs reached over unknown request"
                "by %d\r\n", pid);
            status = acknowledge(-1, NULL, 0);
            break;
        }
        assert(status == 0);
    }
}
