#ifndef STRING_H
#define STRING_H

#include <stddef.h>

int   memcmp (const void *, const void *, size_t);
void *memcpy (void *dst, const void *src, size_t);
void *memmove(void *dst, const void *src, size_t);
void *memset (void *dst, int value, size_t);

size_t strlen (const char *);
size_t strsize(const char *);
int strcmp(const char * str1, const char * str2);
char * strcat(const char *, const char *);
char * filename_join(const char *, const char *);

char * strerror (int errno);

#endif
