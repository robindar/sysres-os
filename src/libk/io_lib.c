#include "io_lib.h"
#include "sys.h"
#include "../usr/io.h"
#include "../proc/proc.h"
#include "string.h"
#include "uart.h"
#include <stdarg.h>

void io_putc(unsigned char c){
    io_request_t request = {.code = 0, .data = (uint64_t) c};
    send(IO_MANAGER_PID, &request, sizeof(request), NULL, 0, true);
}

int io_puts(const char* str){
    int size = strsize(str);
    if(size > IO_BUFF_SIZE)
        /* TODO: set errno */
        return -1;
    io_request_t request;
    memmove(request.buff, str, size);
    request.code = 2;
    return send(IO_MANAGER_PID, &request, sizeof(request), NULL, 0, true);
}

/* Size means strlen + 1 ie with the '\0' */
int io_get_string(char * buff, size_t size){
    io_request_t request = {.code = 3};
    io_response_t response;
    int status =
        send(IO_MANAGER_PID, &request, sizeof(request),
             &response, sizeof(response), true);
    if(status >= 0){
        size_t size_resp = strsize(response.buff);
        memmove(buff, response.buff, size_resp > size ? size : size_resp);
        if(size_resp > size){
            /* Add to lost characters */
            status += size_resp - size;
            /* Make sure there is a \0 */
            buff[size - 1] = '\0';
        }
    }
    return status;
}

int io_put_int(int64_t x, unsigned int base, bool unsign, bool upper_hexa){
    io_request_t request = {.code = 1, .data = (uint64_t) x};
    if(base > (1 << 8) - 1)
        return -1;
    request.buff[0] = (char) base;
    request.buff[1] = (char) unsign;
    request.buff[2] = (char) upper_hexa;
    return send(IO_MANAGER_PID, &request, sizeof(request), NULL, 0, true);
}


/* uart_printf :
 * Aims at mimicking the behaviour of the C printf but for GPIO
 * Doesn't yet support all the options : for now only conversion flags :
 * d,o,u,x,X,c,s,b%*/
int io_printf(const char* format,...) {
    va_list adpar;
    va_start(adpar, format);
    int written = internal_printf(format, adpar, 0, io_putc, io_puts, io_put_int);
    va_end(adpar);
    return written;
}

/* The return value is the number of char written, including the label */
int io_verbose(const char* format __attribute__((__unused__)),...) {
    int written = 0;
#if LOG_LEVEL >= _LOG_VERBOSE_
    va_list adpar;
    va_start(adpar, format);
    io_puts("[VERBOSE] ");
    written = internal_printf(format, adpar, 1, io_putc, io_puts, io_put_int);
    va_end(adpar);
#endif
    return written;
}

int io_debug(const char* format __attribute__((__unused__)),...) {
    int written = 0;
#if LOG_LEVEL >= _LOG_DEBUG_
    va_list adpar;
    va_start(adpar, format);
    io_puts("[ DEBUG ] ");
    written = internal_printf(format, adpar, 1, io_putc, io_puts, io_put_int);
    va_end(adpar);
#endif
    return written;
}

int io_info(const char* format __attribute__((__unused__)),...) {
    int written = 0;
#if LOG_LEVEL >= _LOG_INFO_
    va_list adpar;
    va_start(adpar, format);
    io_puts("[ INFO  ] ");
    written = internal_printf(format, adpar, 1, io_putc, io_puts, io_put_int);
    va_end(adpar);
#endif
    return written;
}

int io_warning(const char* format __attribute__((__unused__)),...) {
    int written = 0;
#if LOG_LEVEL >= _LOG_WARNING_
    va_list adpar;
    va_start(adpar, format);
    io_puts("[WARNING] ");
    written = internal_printf(format, adpar, 1, io_putc, io_puts, io_put_int);
    va_end(adpar);
#endif
    return written;
}

int io_error(const char* format __attribute__((__unused__)),...) {
    int written = 0;
#if LOG_LEVEL >= _LOG_ERROR_
    va_list adpar;
    va_start(adpar, format);
    io_puts("[ ERROR ] ");
    written = internal_printf(format, adpar, 1, io_putc, io_puts, io_put_int);
    va_end(adpar);
#endif
    return written;
}

int io_wtf(const char* format __attribute__((__unused__)),...) {
    int written = 0;
#if LOG_LEVEL >= LOG_WTF
    va_list adpar;
    va_start(adpar, format);
    io_puts("[  WTF  ] ");
    written = internal_printf(format, adpar, 1, io_putc, io_puts, io_put_int);
    va_end(adpar);
#endif
    return written;
}
