#ifndef __LED_H
#define __LED_H

#define LED_RED GPIO_PIN_6      // PC6
#define LED_GREEN GPIO_PIN_9    // PC9
#define LED_BLUE GPIO_PIN_7     // PC7
#define LED_ORANGE GPIO_PIN_8   // PC8

#include <stdint.h>

void InitLEDs(uint32_t leds);

#endif