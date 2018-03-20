#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../misc.h"

#ifndef UART_H
#define UART_H

void uart_init();
void uart_putc(unsigned char c);
unsigned char uart_getc();
int uart_puts(const char* str);
int uart_printf(const char* format,...);

#endif
