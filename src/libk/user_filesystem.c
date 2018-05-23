#include "user_filesystem.h"
#include "../usr/fs_manager.h"
#include "../proc/proc.h"
#include "sys.h"

int uclose (int file_descriptor){
    fs_request_t req = {.code = 0, .file_descriptor = file_descriptor};
    return send(FS_MANAGER_PID, &req, sizeof(fs_request_t), NULL, 0, true);
}

int uopen (const char * path){
    size_t size = strsize(path);
    if(size > IO_BUFF_SIZE){
        err.no = SEND_DATA_TOO_LARGE;
        return -1;
    }
    fs_request_t req = {.code = 1};
    memmove(req.buff1, path, size);
    return send(FS_MANAGER_PID, &req, sizeof(fs_request_t), NULL, 0, true);
}

void useek (int file_descriptor, int offset, enum seek_t whence){
    fs_request_t req = {.code = 2, .file_descriptor = file_descriptor,
                        .data1 = (uint64_t) offset, .data2 = (uint64_t) whence};
    send(FS_MANAGER_PID, &req, sizeof(fs_request_t), NULL, 0, true);
    return;
}

size_t uread  (int file_descriptor,       void * buf, size_t byte_count){
    fs_request_t req = {.code = 3, .file_descriptor = file_descriptor,
                        .data1 = (uint64_t) byte_count};
    fs_response_t resp = {0};
    int size = send(FS_MANAGER_PID, &req, sizeof(fs_request_t),
                      &resp, sizeof(fs_response_t), true);
    memmove(buf, resp.buff, size);
    return size;
}
size_t uwrite (int file_descriptor, const void * buf, size_t byte_count){
    fs_request_t req = {.code = 4, .file_descriptor = file_descriptor,
                        .data1 = (uint64_t) byte_count};
    if(byte_count > IO_BUFF_SIZE){
        err.no = SEND_DATA_TOO_LARGE;
        return -1;
    }
    memmove(req.buff1, buf, byte_count);
    return send(FS_MANAGER_PID, &req, sizeof(fs_request_t), NULL, 0, true);
}

int umove (const char * src_path, const char * dst_path){
    fs_request_t req = {.code = 5};
    size_t size = strsize(src_path);
    if(size > IO_BUFF_SIZE)
        /* TODO: errno */
        return -1;
    memmove(req.buff2, src_path, size);
    size = strsize(dst_path);
    if(size > IO_BUFF_SIZE)
        /* TODO: errno */
        return -1;
    memmove(req.buff2, dst_path, size);
    return send(FS_MANAGER_PID, &req, sizeof(fs_request_t), NULL, 0, true);
}
