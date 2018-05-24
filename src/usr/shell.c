#include "shell.h"
#include "../libk/io_lib.h"
#include "io.h"
#include "../libk/uart.h"
#include "../libk/debug.h"
#include "../libk/string.h"
#include "../test/test.h"
#include "../libk/filesystem.h"

void execute_command(char * str){
    switch(str[0]){
        case 'e':
            /* echo */
            if(memcmp((void*) str, (void *) "echo ", 5) != 0)
                goto FAIL;
            uart_printf(&str[5]);
            break;
        case 't':
            /* test */
            if(memcmp((void*) str, (void *) "test ", 5) != 0)
                goto FAIL;
            run_test(str[5]);
            break;
        default:
        FAIL:
            uart_error("Shell: Unknown command\r\n");
    }
}

void run_test(char c){
    switch(c){
    case '0':
        time_send();
        break;
    case '1':
        print_io_formatting_tests();
        test_io_get_string(5);
        break;
    case '2':
        /* send(FS_MANAGER_PID, , size_t send_size, void *ack_data, size_t ack_size, bool wait) */
        print_filesystem_info();
        break;
    case '3':
        fs_test1();
        break;
    case '4':
        fs_test2();
        break;
    case '5':
        a_page_s_file();
        break;
    case '6':
        sched_test1();
        break;
    case '7':
        asm volatile("TLBI ALLE1");
        break;
    }
}


__attribute__((__noreturn__))
void shell(){
    char buff[IO_BUFF_SIZE];
    int status;
    while(1){
        uart_puts("\r\n[root@raspi /]# ");
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
