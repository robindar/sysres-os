#ifndef ERRNO_H
#define ERRNO_H
#include <stdint.h>
/* ERRNO
 *
 * Upper 16 bits are the error number
 * Lower 16 bits are the additional information
 *
 * i.e.
 * errno = EACCESS;
 * errno = ECUSTOMERROR | 83;
 */
/* Screw up good practices, nothing else worked */

typedef enum {
    OK = 0,
    STACK_OVERFLOW,
    HEAP_OVERFLOW,
    SEG_FAULT,
    IO,
    ACCESS,
    IS_DIRECTORY,
    NOT_DIRECTORY,
    ARG_INVALID,
    FILE_EXIST,
    FILE_MISSING,
    DIRECTORY_MISSING,
    FILE_BUSY,
    NO_SPACE_LEFT,
    MAX_PROC_REACHED,
    TOO_HIGH_CHILD_PRIORITY,
    NO_CHILD,
    INVALID_PID,
    TARGET_NOT_LISTENING,
    SOURCE_ALREADY_ACKNOWLEDGED,
} errno_t;

typedef uint64_t errdata_t;

typedef struct {
    errno_t no;
    errdata_t data;
} err_t;

extern err_t err;

void print_err(err_t);
#endif
