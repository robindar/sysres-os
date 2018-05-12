#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "string.h"
#include "../memory/alloc.h"

void init_filesystem ();

int close (int file_descriptor);
int open (const char * path, int oflag);
size_t read  (int file_descriptor,       void * buf, size_t byte_count, size_t offset);
size_t write (int file_descriptor, const void * buf, size_t byte_count, size_t offset);

#endif
