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

enum errno_value {
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
    NO_SPACE_LEFT
};

struct errno_struct {
    enum errno_value no;
    uint64_t descr;
};

extern struct errno_struct err;

#endif
