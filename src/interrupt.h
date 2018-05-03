#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "stdint.h"
#include "libc/misc.h"
#include "libc/uart/uart.h"
#include "libc/debug/debug.h"
#include "memory/mmu.h"
#include "stdbool.h"
void display_esr_eln_info(uint64_t esr_eln);
void display_pstate_info(uint64_t pstate);

#endif
