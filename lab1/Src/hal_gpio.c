#include <stdint.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>
#include <stm32f072xb.h>

void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    // Configure Red and Blue LED pins (PC6-PC7)
    GPIOC->MODER = 0x05000; // General purpose output mode
    GPIOC->OTYPER = 0x0;    // Push-pull output type
    GPIOC->OSPEEDR = 0x0;   // Low speed
    GPIOC->PUPDR = 0x0;     // No pull-up or pull-down

    // Configure USER button pin PA0
    //GPIOA->MODER = 0x0;     // Digital input mode
    //GPIOA->OSPEEDR = 0x0;   // Low speed
    //GPIOA->PUPDR = 0x2;     // Pull-down resistor
}


/*

*/


void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    GPIOx->ODR &= ~GPIO_Pin;                // Reset pin

    if (PinState)
    {
        GPIOx->ODR |= (GPIO_Pin & 0xffffffff);    // Set pin
    }
}


void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->ODR ^= GPIO_Pin;
}
