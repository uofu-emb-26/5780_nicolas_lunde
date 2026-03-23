#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>

void ADC_Init(uint8_t channel);
int ADC_InitPin(uint8_t channel);

#endif