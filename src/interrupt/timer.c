
#include <stdint.h>
#include "../libk/misc.h"
#include "../libk/uart.h"
#include "timer.h"
#include "../libk/debug.h"
#include <stdbool.h>


/* This work relies on the BCM 2835 datasheet modified to fit the BCM 2837 (as there is no official BCM2837 doc...) */
/* https://github.com/raspberrypi/documentation/issues/325#issuecomment-379651504 */
/* The adresses are confirmed by https://ultibo.org/wiki/Unit_BCM2837 */


/* 32 bits registers */
#define AT32(addr) (*((uint32_t *) (addr)))
#define BASE                           0x3f000000

#define INTERRUPT_BASE              BASE + 0xb000
#define IRQ_BASIC_PENDING INTERRUPT_BASE + 0x0200
#define IRQ_PENDING_1     INTERRUPT_BASE + 0x0204
#define IRQ_PENDING_2     INTERRUPT_BASE + 0x0208
#define FIQ_CONTROL       INTERRUPT_BASE + 0x020c
#define ENABLE_IRQ_1      INTERRUPT_BASE + 0x0210
#define ENABLE_IRQ_2      INTERRUPT_BASE + 0x0214
#define ENABLE_BASIC_IRQ  INTERRUPT_BASE + 0x0218
#define DISABLE_IRQ_1     INTERRUPT_BASE + 0x021c
#define DISABLE_IRQ_2     INTERRUPT_BASE + 0x0220
#define DISABLE_BASIC_IRQ INTERRUPT_BASE + 0x0224

#define TIMER_BASE                  BASE + 0xb400
#define TIMER_LOAD            TIMER_BASE + 0x0000
#define TIMER_VALUE           TIMER_BASE + 0x0004
#define TIMER_CTRL            TIMER_BASE + 0x0008
#define TIMER_IRQ_CLEAR_ACK   TIMER_BASE + 0x000C
#define TIMER_RAW_IRQ         TIMER_BASE + 0x0010
#define TIMER_MSK_IRQ         TIMER_BASE + 0x0014
#define TIMER_RELOAD          TIMER_BASE + 0x0018
#define TIMER_PRE_DIV         TIMER_BASE + 0x001C
#define TIMER_FREE_RUNNING    TIMER_BASE + 0x0020

/* Approximate : the value given by the clock */
/* ie half the syst clock divided by the pre divider + 1 does not match */

/* Indiactes whether the timer is supposed to be running */
static bool timer_on = false;

void init_timer_irq(){
    uart_verbose("Beginning timer and IRQ initiliazation\r\n");
    AT32(ENABLE_BASIC_IRQ) |= 1;
    asm volatile("msr DAIFSet,#2"); /* Timer are not effective ate EL1 */
    AT32(TIMER_PRE_DIV) = 0x7d; /* default */
    uint64_t reg;
    /* Enable access to the physical counter at EL0 */
    asm volatile("mrs %0, CNTKCTL_EL1":"=r"(reg)::);
    reg |= 1;
    asm volatile("msr CNTKCTL_EL1, %0": :"r"(reg):);
    uart_verbose("Done timer and IRQ initiliazation\r\n");
}

void start_countdown(uint32_t countdown){
    #ifdef NO_TIMER
    #ifndef HARDWARE
    uart_error("WARNING : you are launching a timer on Qemu but it does not work\r\n");
    #endif
    timer_on = true;
    AT32(TIMER_CTRL)  = 0;
    if(countdown > QUANTUM || countdown < EPSILON)
        AT32(TIMER_LOAD) = EPSILON;
    else
        AT32(TIMER_LOAD) = countdown;
    /* Writing an alleged  Read Only register, thanks Linux for the tip*/
    AT32(TIMER_VALUE) = 0xFFFFFFFF;
    #ifdef PROC_VERBOSE
    uart_verbose("Starting timer\r\n");
    #endif
    AT32(TIMER_CTRL) |=  (1 << 1) | (1 << 7) | (1 << 5);
    return;
    #endif
}

int is_timer_irq(){
    return AT32(IRQ_BASIC_PENDING) & 1;
}

int is_countdown_finished(){
    return AT32(TIMER_RAW_IRQ) & 1;
}

void mask_timer_irq(){
    AT32(TIMER_CTRL) &= ~(1 << 5);
}

uint32_t get_curr_timer_value(){
    return is_countdown_finished() ? 0 : AT32(TIMER_VALUE);
}

void clear_ack_timer_irq(){
    timer_on = false;
    AT32(TIMER_IRQ_CLEAR_ACK) = 0;
    AT32(TIMER_CTRL)  = 0;
}

/* To be used after an excecption but not after a context switch */
/* It tries to take care of errors */
/*(timer expiring during switchs and undetected, timer too low for the switch) */
void restart_timer(){
    #ifdef NO_TIMER
    if(!timer_on) return;
    #ifdef PROC_VERBOSE
    uart_verbose("Restarting timer\r\n");
    #endif
    uint32_t value = AT32(TIMER_VALUE);
    AT32(TIMER_CTRL)  = 0;
    if(value > QUANTUM || value < EPSILON
       || is_countdown_finished()) AT32(TIMER_LOAD) = EPSILON;
    AT32(TIMER_CTRL) |=  (1 << 1) | (1 << 7) | (1 << 5);
    return;
    #endif
}

void print_timer_status(){
    uint64_t daif;
    asm volatile("mrs %0, DAIF":"=r"(daif)::);
    uart_info("Timer status:\r\n"
              "Timer IRQ Pending: %d\r\n"
              "Countdown done: %d\r\n"
              "Countdown: 0x%x\r\n"
              "IRQ in PSTATE: %d\r\n"
              "TIMER_CTRL.ENABLE: %d\r\n"
              "TIMER_CTRL.ENABLE_IRQ: %d\r\n",
              is_timer_irq(), is_countdown_finished(), get_curr_timer_value(),
              (daif & (1 << 7)) >> 7, (AT32(TIMER_CTRL) & 5) >> 5, (AT32(TIMER_CTRL) & 7) >> 7);
}

/* Returns a random number from [0, bound[ */
uint64_t random(uint64_t bound){
    uint64_t val;
    asm("mrs %0, CNTPCT_EL0" : "=r"(val)::);
    return (val % bound);
}

#define NON_DETERMINISTIC
uint64_t random_law(unsigned int * law, int n){
    #ifdef DETERMINISTIC_MAX
    /* we assume there is at least one proc */
    unsigned int max_val = law[0];
    int max = 0;
    for(int i = 1; i < n; i++){
        if(max_val < law[i]){
            max_val = law[i];
            max = i;
        }
    }
    return max;
    #endif
    #ifdef DETERMINISTIC_MIN
    /* we assume there is at least one proc */
    unsigned int min_val = ~((unsigned int)0);
    int min = 0;
    for(int i = 0; i < n; i++){
        if(min_val > law[i] && law[i] > 0){
            min_val = law[i];
            min = i;
        }
    }
    return min;
    #endif
    #ifdef NON_DETERMINISTIC
    uint64_t total = 0;
    for(int i = 0; i < n; i++) total += law[i];
    uint64_t val = random(total);
    uint64_t sum = 0;
    for(int i = 0; i < n; i++){
        sum += law[i];
        if(sum > val)
            return i;
    }
    /* Useless */
    return (n - 1);
    #endif
}
