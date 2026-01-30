#include "hal_rcc.h"
#include <stm32f072xb.h>

int HAL_RCC_GPIOC_CLK_Enable()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN;
}