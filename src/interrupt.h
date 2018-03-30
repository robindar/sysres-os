#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "stdint.h"

void display_esr_eln_info(uint64_t esr_eln);
void display_pstate_info(uint64_t pstate);

#endif
