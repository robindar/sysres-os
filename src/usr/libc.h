#ifndef LIBC_H
#define LIBC_H

#include "../libk/string.h"

/* Temporary */
#define SYSCALL(code) asm volatile("SVC #" # code "")

#endif
