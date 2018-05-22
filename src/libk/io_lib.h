#ifndef IO_LIB_H
#define IO_lIB_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
void io_putc(unsigned char c);
int io_puts(const char* str);
int io_get_string(char * buff, size_t size);
int io_put_int(int64_t x, unsigned int base, bool unsign, bool upper_hexa);
int io_printf(const char* format,...);
int io_verbose(const char* format,...);
int io_debug(const char* format,...);
int io_info(const char* format,...);
int io_warning(const char* format,...);
int io_error(const char* format,...);
int io_wtf(const char* format,...);
#endif
