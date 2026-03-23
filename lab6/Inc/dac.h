#ifndef __DAC_H
#define __DAC_H

#include <stdint.h>

void DAC_Init(uint8_t channel);
int DAC_InitPin(uint8_t channel);

#endif