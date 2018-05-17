#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

#define TIMER_IRQ_PAGE     0x3f00b000
#define SECOND                2040816
#define EPSILON                   200
#define QUANTUM            (SECOND/1)


void init_timer_irq();
void start_countdown(uint32_t count);
int is_timer_irq();
int is_countdown_finished();
void clear_ack_timer_irq();
void print_timer_status();
uint32_t get_curr_timer_value();

#endif
