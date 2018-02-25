#ifndef _STRING_H
#define _STRING_h

#include <stddef.h>

int   memcmp (const void *, const void *, size_t);
void *memcpy (void *dst, const void *src, size_t);
void *memmove(void *dst, const void *src, size_t);
void *memset (void *dst, int value, size_t);

#endif
