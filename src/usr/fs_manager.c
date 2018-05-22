#include "fs_manager.h"
#include "../libk/sys.h"
#include "../libk/filesystem.h"

#define FS_VERBOSE
#ifdef FS_VERBOSE
#define fs_verbose(...) uart_verbose(__VA_ARGS__)
#else
#define fs_verbose(...) ((void) 0)
#endif

__attribute__((__noreturn__))
void main_fs_manager(){
    assert(get_curr_pid() == FS_MANAGER_PID);
    int pid, status = 0;
    fs_request_t request = {0};
    fs_response_t response = {0};
    while(1){
        pid = receive(&request, sizeof(fs_request_t));
        switch(request.code){
        case 0:
            fs_verbose("FS: Fclose from %d\r\n", pid);
            status = fclose(request.file_descriptor);
            status = acknowledge(status, NULL, 0);
            break;
        case 1:
            fs_verbose("FS: Fopen from %d\r\n", pid);
            status = fopen(request.buff1, (int) request.data1);
            status = acknowledge(status, NULL, 0);
            break;
        case 2:
            fs_verbose("FS: Fseek from %d\r\n", pid);
            fseek(request.file_descriptor, (int)request.data1,
                  (enum seek_t) request.data2);
            status = acknowledge(0, NULL, 0);
            break;
        case 3:
            fs_verbose("FS: Fread from %d\r\n", pid);
            status = fread(request.file_descriptor, response.buff, request.data1);
            /* we could pass size = IO_BUFF_SIZE to save some space */
            status = acknowledge(status, &response, sizeof(fs_response_t));
            break;
        case 4:
            fs_verbose("FS: Fwrite from %d\r\n", pid);
            status = fwrite(request.file_descriptor, request.buff1, request.data1);
            status = acknowledge(status, NULL, 0);
            break;
        case 5:
            fs_verbose("FS: move from %d\r\n", pid);
            move(request.buff1, request.buff2);
            status = acknowledge(status, NULL, 0);
            break;
        default:
            /* Unknown signal */
            uart_error(
                "FS processs reached over unknown request"
                "by %d\r\n", pid);
            status = acknowledge(-1, NULL, 0);
            break;
        }
        assert(status == 0);
    }
}
