#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "misc.h"

#ifndef UART_H
#define UART_H

#define GPIO_BASE       0x3F200000


#define _LOG_WTF_     0
#define _LOG_ERROR_   1
#define _LOG_WARNING_ 2
#define _LOG_INFO_    3
#define _LOG_DEBUG_   4
#define _LOG_VERBOSE_ 5

#define _LOG_F_  _LOG_WTF_
#define _LOG_E_  _LOG_ERROR_
#define _LOG_W_  _LOG_WARNING_
#define _LOG_I_  _LOG_INFO_
#define _LOG_D_  _LOG_DEBUG_
#define _LOG_V_  _LOG_VERBOSE_

#define EMPTY()
#define DEFER(id) id EMPTY()
#define EXPAND(...) __VA_ARGS__
#define LOG_EXPAND(n) _LOG_ ## n ## _

#ifdef LOG
#define LOG_LEVEL EXPAND(DEFER(LOG_EXPAND) (LOG))
#else
#define LOG_LEVEL _LOG_WARNING_
#endif

void uart_send(unsigned int t);

void uart_init();
void uart_putc(unsigned char c);
unsigned char uart_getc();
int uart_puts(const char* str);
int uart_printf(const char* format,...);

/* When to use each logging level (shortcut between brackets):
 *
 * VERBOSE [V] : Absolutely crazy logging for desperate situations only
 * DEBUG   [D] : Unusual logging required only during debugging
 * INFO    [I] : Useful, non-critical information
 * WARNING [W] : Likely-unwanted behavior reporting
 * ERROR   [E] : Error detected/caught
 * WTF     [F] : (What a Terrible Failure) Violation of the laws of logic
 *               i.e. contradiction of previous assertion, should NEVER happen
 */

int uart_verbose(const char* format,...);
int uart_debug(const char* format,...);
int uart_info(const char* format,...);
int uart_warning(const char* format,...);
int uart_error(const char* format,...);
int uart_wtf(const char* format,...);

int uart_get_string(char * buff, size_t size);


#endif
