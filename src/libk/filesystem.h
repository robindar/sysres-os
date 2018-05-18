#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "string.h"
#include "../memory/alloc.h"

void init_filesystem ();

int fclose (int file_descriptor);
int fopen (const char * path, int oflag);
size_t fread  (int file_descriptor,       void * buf, size_t byte_count, size_t offset);
size_t fwrite (int file_descriptor, const void * buf, size_t byte_count, size_t offset);

#endif
