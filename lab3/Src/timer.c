#include "timer.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f072xb.h"

void TIM2_IRQHandler(void);

void Timer4Hz_Init(void)
{
    // Enable timer 2 
    RCC->APB1ENR = RCC_APB1ENR_TIM2EN;

    /**
     * 8 MHz input clock / (7999 + 1) = 1 KHz
     * 1 / 1 KHz = 1 ms. 1 ms * 250 = 250 ms
     * 1 / 250 ms = 4 Hz
     */
    TIM2->ARR = 250; // Count up to 250
    TIM2->PSC = 7999;

    TIM2->DIER |= TIM_DIER_UIE; // Enable update interrupt
    TIM2->CR1 |= TIM_CR1_CEN; // Enable timer 2. Default upcounter.
    
    NVIC_EnableIRQ(TIM2_IRQn);       // Enable EXTI0 line in NVIC
    NVIC_SetPriority(TIM2_IRQn, 3);  // Set high priority
}

void TimerPWM_Init(void)
{
    // Enable timer 3 clock 
    RCC->APB1ENR = RCC_APB1ENR_TIM3EN; 

    /**
     * 8 MHz input clock / (7 + 1) = 1 KHz
     * 1 / 8 KHz = 125 us. 125 us * 10 = 1250 us
     * 1 / 1250 us = 800 Hz
     */
    TIM3->ARR = 10000; // Count up to 10
    TIM3->PSC = 0;

    uint16_t temp = TIM3->CCMR1;
    temp &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_CC2S | TIM_CCMR1_OC1M | TIM_CCMR1_CC1S);  
    // CCS2S and CCS1s are now 0, which is output mode
    temp |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0); // Set channel 1 output mode to PWM mode 2
    temp |= (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1); // Set channel 2 output mode to PWM mode 1
    temp |= (TIM_CCMR1_OC2PE | TIM_CCMR1_OC1PE); // Preload registers for both channels
    TIM3->CCMR1 = temp;

    TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E; // Output enable

    /**
     * ARR = 10000
     * 20% of 10000 = 2000
     * Generates output when timer counter reaches 250
     */
    TIM3->CCR1 = 2000;
    TIM3->CCR2 = 2000;

    TIM3->CR1 |= TIM_CR1_CEN; // Enable timer 3. Default upcounter.

    /*NVIC_EnableIRQ(TIM3_IRQn);       // Enable EXTI0 line in NVIC
    NVIC_SetPriority(TIM3_IRQn, 3);  // Set high priority*/
}

void TIM2_IRQHandler(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    TIM2->SR &= ~(0x1);

    static uint16_t x = 0;
    static int up = 1;
    TIM3->CCR1 = x;
    if (x == 100)
        up = 0;
    else
        up = 1;
    
    if (up)
        x++;
    else
        x--;
}