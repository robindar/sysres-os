#ifndef DEBUG_H
#define DEBUG_H

#include "stdint.h"

#define print_reg(reg) \
  uint64_t __variable__print__reg__ = 0;\
  asm volatile("MRS %0, " #reg : "=r"(__variable__print__reg__) : :);\
  uart_info("Reg " #reg " : 0x%x\r\n", __variable__print__reg__);

void abort();
void assert(int bl);

#endif
