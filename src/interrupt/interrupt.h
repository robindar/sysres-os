#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>
#include <stdint.h>
#include "../libk/debug.h"
#include "../libk/misc.h"
#include "../libk/uart.h"
#include "../memory/mmu.h"
#include "../proc/proc.h"
#include "timer.h"
void display_esr_eln_info(uint64_t esr_eln);
void display_pstate_info(uint64_t pstate);

#endif
