#include "errno.h"
#include "uart.h"
err_t err = {0};

void print_err(err_t e){
    uart_verbose("Printing err_t:\r\nerr.no : %d\r\nerr.data : 0x%x\r\n",
                 e.no, e.data);
}
