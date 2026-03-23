#include "led.h"
#include "stm32f0xx_hal.h"

void InitLEDs(uint32_t leds)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN; 
    GPIO_InitTypeDef initStr = {leds, 
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &initStr); 
}