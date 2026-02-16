#ifndef __TIMER_H
#define __TIMER_H

#include "stdint.h"

void Timer4Hz_Init(void);
void TimerPWM_Init(uint16_t arr, uint16_t psc, uint16_t ccr);

#endif