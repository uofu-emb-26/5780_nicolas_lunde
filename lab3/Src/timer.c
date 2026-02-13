#include "timer.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f072xb.h"

void TIM2_IRQHandler(void);

void Timer4Hz_Init(void)
{
    // Enable timer 2 and 3 clocks. Using timer 2
    RCC->APB1ENR = RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN;
    TIM2->ARR = 250; // Count up to 250

    /**
     * 8 MHz input clock / (7999 + 1) = 1 KHz
     * 1 / 1 KHz = 0.1 ms. 0.1 ms * 250 = 0.25 ms
     * 1 / 0.25 ms = 4 Hz
     */
    TIM2->PSC = 7999;
    TIM2->DIER |= TIM_DIER_UIE; // Enable update interrupt
    TIM2->CR1 |= TIM_CR1_CEN; // Enable timer 2. Default upcounter.
    
    NVIC_EnableIRQ(TIM2_IRQn);       // Enable EXTI0 line in NVIC
    NVIC_SetPriority(TIM2_IRQn, 3);  // Set high priority
}

void TIM2_IRQHandler(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    TIM2->SR &= ~(0x1);
}