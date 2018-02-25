#include "string.h"

int memcmp (const void * pa, const void * pb, size_t size)
{
	const unsigned char *a = (const unsigned char *) pa;
	const unsigned char *b = (const unsigned char *) pb;
	for (size_t i=0; i<size; i++)
		if (a[i] != b[i])
			return (a[i] < b[i] ? -1 : 1);
	return 0;
}

void * memcpy (void * pdst, const void * psrc, size_t size)
{
	unsigned char * dst = (unsigned char *) pdst;
	const unsigned char * src = (const unsigned char *) psrc;
	for (size_t i=0; i<size; i++)
		dst[i] = src[i];
	return pdst;
}

void * memmove (void * pdst, const void * psrc, size_t size)
{
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

void * memset (void * pdst, int v, size_t size)
{
	unsigned char * dst = (unsigned char *) pdst;
	for (size_t i=0; i<size; i++)
		dst[i] = (unsigned char) v;
	return pdst;
}

size_t strlen (const char * str)
{
	size_t len = 0;
	while (str[len]) { len++; }
	return len;
}
