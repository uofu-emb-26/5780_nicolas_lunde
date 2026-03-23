#include "dac.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "uart.h"

void DAC_Init(uint8_t channel)
{
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC1->SWTRIGR = (1 << (channel - 1));
    DAC1->CR |= DAC_CR_EN1;
}

int DAC_InitPin(uint8_t channel)
{
    GPIO_InitTypeDef initStr = {
        .Mode = GPIO_MODE_ANALOG, 
        .Pull = GPIO_NOPULL};

    switch (channel)
    {
    case 1:
        initStr.Pin = GPIO_PIN_4;
        break;
    case 2:
        initStr.Pin = GPIO_PIN_5;
        break;
    default:
        return -1; 
    }

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 
    HAL_GPIO_Init(GPIOA, &initStr); 
    return 0;
}