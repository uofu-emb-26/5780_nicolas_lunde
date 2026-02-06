#include <stdint.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>
#include <stm32f072xb.h>

void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    uint32_t pin = 0;
    uint32_t pin2 = 0;
    uint32_t temp;

    while (pin < 16)
    {
        if ((0x1 << pin) & GPIO_Init->Pin)
        {

            temp = GPIOx->MODER;
            temp &= ~(0x3 << pin2);
            temp |= (GPIO_Init->Mode & GPIO_MODE) << pin2;
            GPIOx->MODER = temp;

            temp = GPIOx->OTYPER;
            temp &= ~(0x1 << pin);
            temp |= ((GPIO_Init->Mode & OUTPUT_TYPE) >> OUTPUT_TYPE_Pos) << pin;
            GPIOx->OTYPER = temp;

            temp = GPIOx->OSPEEDR;
            temp &= ~(0x3 << pin2);
            temp |= GPIO_Init->Speed << pin2;
            GPIOx->OSPEEDR = temp;

            temp = GPIOx->PUPDR;
            temp &= ~(0x3 << pin2);
            temp |= GPIO_Init->Pull << pin2;
            GPIOx->PUPDR = temp;
        }

        pin++; 
        pin2 += 2;
    }
}


GPIO_PinState My_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    uint32_t debouncer = 0;
    while (1)
    {
        debouncer = (debouncer << 1);

        if (GPIOx->IDR & GPIO_Pin)
        {
            debouncer |= 0x1;
        } 
        
        if (debouncer == 0xFFFFFFFF)
        {
            return GPIO_PIN_SET;
        }

        if (debouncer == 0x00000000)
        {
            return GPIO_PIN_RESET;
        }

        if (debouncer == 0x7FFFFFFF)
        {
            return GPIO_PIN_SET;
        }
    }
}


void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    GPIOx->ODR &= ~GPIO_Pin;                // Reset pin

    if (PinState)
    {
        GPIOx->ODR |= (GPIO_Pin & 0xFFFFFFFF);    // Set pin
    }
}


void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->ODR ^= GPIO_Pin;
}

void ButtonInt_Init(void)
{
    EXTI->IMR |= 0x1;       // Enable interrupt for EXTI0
    EXTI->RTSR |= 0x1;      // Interrupt on rising edge trigger
    SYSCFG->EXTICR[0] |= 0x0; // EXTI0 will take interrupt input from PA0
    NVIC_EnableIRQ(EXTI0_1_IRQn);       // Enable EXTI0 line in NVIC
    NVIC_SetPriority(EXTI0_1_IRQn, 1);  // Set high priority
}