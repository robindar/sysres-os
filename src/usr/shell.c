#include "shell.h"
#include "../libk/io_lib.h"
#include "io.h"
#include "../libk/uart.h"
#include "../libk/debug.h"
#include "../libk/string.h"

void execute_command(char * str){
    switch(str[0]){
        case 'c':
            /* cat */
            break;
        case 'e':
            /* echo */
            if(memcmp((void*) str, (void *) "echo ", 5) != 0)
                goto FAIL;
            uart_printf(&str[5]);
        default:
        FAIL:
            uart_error("Shell: Unknown command\r\n");
    }
}


__attribute__((__noreturn__))
void shell(){
    char buff[IO_BUFF_SIZE];
    int status;
    while(1){
        uart_putc(':');
        status = uart_get_string(buff, IO_BUFF_SIZE);
        assert(status >= 0);
        if(status > 0){
            uart_warning("Shell: %d characters lost\r\n");
            continue;
        }
        if(strlen(buff) > 0)
            execute_command(buff);
    }
}
