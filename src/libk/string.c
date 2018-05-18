#include "misc.h"
#include "string.h"
#include "../memory/alloc.h"

int memcmp (const void * pa, const void * pb, size_t size) {
    const unsigned char *a = (const unsigned char *) pa;
    const unsigned char *b = (const unsigned char *) pb;
    for (size_t i=0; i<size; i++)
        if (a[i] != b[i])
            return (a[i] < b[i] ? -1 : 1);
    return 0;
}

void * memcpy (void * pdst, const void * psrc, size_t size) {
    unsigned char * dst = (unsigned char *) pdst;
    const unsigned char * src = (const unsigned char *) psrc;
    for (size_t i=0; i<size; i++)
        dst[i] = src[i];
    return pdst;
}

void * memmove (void * pdst, const void * psrc, size_t size) {
    unsigned char * dst = (unsigned char *) pdst;
    const unsigned char * src = (const unsigned char *) psrc;
    if (dst < src)
        for (size_t i=0; i<size; i++)
            dst[i] = src[i];
    else
        for (size_t i=size; i--;)
            dst[i] = src[i];
    return pdst;
}

void * memset (void * pdst, int v, size_t size) {
    unsigned char * dst = (unsigned char *) pdst;
    for (size_t i=0; i<size; i++)
        dst[i] = (unsigned char) v;
    return pdst;
}

size_t strlen (const char * str) {
    size_t len = 0;
    while (str[len]) { len++; }
    return len;
}

size_t strsize(const char * str){
    return strlen(str) + 1;
}

int strcmp(const char * str1, const char * str2){
    size_t i = 0;
    while((str1[i] != '\0') && (str2[i] != '\0')){
        if (str1[i] != str2[i])
            return (str1[i] < str2[i] ? -1 : 1);
        i++;
    }
    if (str1[i] != str2[i])
        return (str1[i] == '\0' ? -1 : 1);
    return 0;
}

char * strerror (int errno) {
    switch((errno >> 16) & MASK(15,0)) {
        default:
          return "Undefined ERRNO";
    }
}

char * strcat(const char * str1, const char * str2) {
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char * cat = kmalloc( (len1 + len2 + 1) * sizeof(char) );
    memcpy(cat, str1, len1);
    memcpy(cat + len1, str2, len2 + 1);
    return cat;
}

char * filename_join(const char * str1, const char * str2) {
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char * cat = kmalloc( (len1 + len2 + 2) * sizeof(char) );
    memcpy(cat, str1, len1);
    memcpy(cat + len1 + 1, str2, len2 + 1);
    cat[len1] = '/';
    return cat;

}
