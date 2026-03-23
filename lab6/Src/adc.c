#include "adc.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "uart.h";

void ADC_Init(uint8_t channel)
{


    RCC->APB2ENR |= RCC_APB2ENR_ADCEN; 
    RCC->CR2 |= RCC_CR2_HSI14ON; // HSI14 clock for ADC
    while (!(RCC->CR2 & RCC_CR2_HSI14RDY)) {}
    ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE; // Select HSI14 clock

    // Make sure ADC is disabled
    while (ADC1->CR & ADC_CR_ADEN)
    {
        ADC1->CR |= ADC_CR_ADDIS;
    }

    ADC1->CFGR1 = 0;  // Reset config (important)
    ADC1->CFGR1 |= (0x2 << ADC_CFGR1_RES_Pos); // 8-bit resolution
    ADC1->CHSELR = ADC_CHSELR_CHSEL0;
    ADC1->CFGR1 |= ADC_CFGR1_CONT;               // Continous conversion mode
 
    // Calibration
    if (ADC1->CR & ADC_CR_ADEN)
    {
        ADC1->CR |= ADC_CR_ADDIS;
    }
    while (ADC1->CR & ADC_CR_ADEN) {}
    ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN;     // DMAEN = 0
    ADC1->CR |= ADC_CR_ADCAL;            // ADCAL = 1
    while (ADC1->CR & ADC_CR_ADCAL) {}   // Wait until ADCAL = 0

    if (ADC1->ISR & ADC_ISR_ADRDY)
    {
        ADC1->ISR |= ADC_ISR_ADRDY;
    }
    ADC1->CR |= ADC_CR_ADEN; // Enable ADC
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {} // Wait until ADC is ready
    ADC1->CR |= ADC_CR_ADSTART;

    UART_TransmitHex(3, ADC1->ISR, 4);
    UART_TransmitString(3, "\r\nCFGR1 (0x00002010): ");
    UART_TransmitHex(3, ADC1->CFGR1, 4);
    UART_TransmitString(3, "\r\nCR (0x00000001): ");
    UART_TransmitHex(3, ADC1->CR, 4);
    UART_TransmitString(3, "\r\nCHSELR (0x00000100): ");
    UART_TransmitHex(3, ADC1->CHSELR, 4);
    UART_TransmitString(3, "\r\nCCR: ");
    UART_TransmitHex(3, ADC->CCR, 4);
}

int ADC_InitPin(uint8_t channel)
{
    static uint32_t pin_map[16] = {
        GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, 
        GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_0, GPIO_PIN_1,
        GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4,
        GPIO_PIN_5
    };

    GPIO_InitTypeDef initStr = {
        .Pin = pin_map[channel], 
        .Mode = GPIO_MODE_ANALOG, 
        .Pull = GPIO_NOPULL};

    if (channel < 8)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 
        HAL_GPIO_Init(GPIOA, &initStr); 
    }
    else if (channel < 10)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN; 
        HAL_GPIO_Init(GPIOB, &initStr); 
    }
    else if (channel < 16)
    {
        RCC->AHBENR |= RCC_AHBENR_GPIOCEN; 
        HAL_GPIO_Init(GPIOC, &initStr); 
    }
    else
        return -1; 
    
    UART_TransmitString(3, "GPIO: ");
    UART_TransmitHex(3, GPIOB->MODER, 4);
    UART_TransmitString(3, "\r\n");
    return 0;
}