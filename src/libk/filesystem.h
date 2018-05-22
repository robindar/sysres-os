#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "string.h"
#include "../memory/alloc.h"

enum seek_t {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

void init_filesystem ();

int fclose (int file_descriptor);
int fopen (const char * path, int oflag);
void fseek    (int file_descriptor, int offset, enum seek_t whence);
size_t fread  (int file_descriptor,       void * buf, size_t byte_count);
size_t fwrite (int file_descriptor, const void * buf, size_t byte_count);

#endif
