#ifndef USER_FILESYSTEM_H
#define USER_FILE_SYSTEM_H
#include "filesystem.h"

int uclose (int file_descriptor);
int ufopen (const char * path, int oflag);
void useek    (int file_descriptor, int offset, enum seek_t whence);
size_t uread  (int file_descriptor,       void * buf, size_t byte_count);
size_t uwrite (int file_descriptor, const void * buf, size_t byte_count);

int umove (const char * src_path, const char * dst_path);
#endif
