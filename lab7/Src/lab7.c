
#include <stdio.h>
#include <stdlib.h>
#include "stm32f0xx.h"
#include "motor.h"
#include "SEGGER_RTT.h"
#include "uart.h"

#define UART_PORT 3

/* ---------------------------------------------------------------------------------------------------------
 *  Global Variable Declarations
 *  -------------------------------------------------------------------------------------------------------------
 */
volatile uint32_t debouncer;

/* -------------------------------------------------------------------------------------------------------------
 *  Miscellaneous Core Functions
 *  -------------------------------------------------------------------------------------------------------------
 */

void LED_init(void) {
    // Initialize PC8 and PC9 for LED's
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;                                          // Enable peripheral clock to GPIOC
    GPIOC->MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;                  // Set PC8 & PC9 to outputs
    GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);                    // Set to push-pull output type
    GPIOC->OSPEEDR &= ~((GPIO_OSPEEDR_OSPEEDR8_0 | GPIO_OSPEEDR_OSPEEDR8_1) |
                        (GPIO_OSPEEDR_OSPEEDR9_0 | GPIO_OSPEEDR_OSPEEDR9_1));   // Set to low speed
    GPIOC->PUPDR &= ~((GPIO_PUPDR_PUPDR8_0 | GPIO_PUPDR_PUPDR8_1) |
                      (GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR9_1));             // Set to no pull-up/down
    GPIOC->ODR &= ~(GPIO_ODR_8 | GPIO_ODR_9);                                   // Shut off LED's
}

void  button_init(void) {
    // Initialize PA0 for button input
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;                                          // Enable peripheral clock to GPIOA
    GPIOA->MODER &= ~(GPIO_MODER_MODER0_0 | GPIO_MODER_MODER0_1);               // Set PA0 to input
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEEDR0_0 | GPIO_OSPEEDR_OSPEEDR0_1);     // Set to low speed
    GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_1;                                        // Set to pull-down
}

/* Called by SysTick Interrupt
 * Performs button debouncing, changes wave type on button rising edge
 * Updates frequency output from ADC value
 */
void Lab7_Systick_Callback(void) {
    // Remember that this function is called by the SysTick interrupt
    // You can't call any functions in here that use delay

    debouncer = (debouncer << 1);
    if(GPIOA->IDR & (1 << 0)) {
        debouncer |= 0x1;
    }

    if(debouncer == 0x7FFFFFFF) {
        // Begin critical section
        __disable_irq();
        switch(target_rpm) {
            case 80:
                target_rpm = 50;
                break;
            case 50:
                target_rpm = 81;
                break;
            case 0:
                target_rpm = 80;
                break;
            default:
                target_rpm = 0;
                break;
        }
        __enable_irq();
        // End critical section
    }
}

void uart_receive(uint8_t port, char c)
{
    static char str[8];
    static int n;

    if (port != UART_PORT)
        return;

    str[n] = c;
    n++;

    if (c == '\r' || n == 8) {
        target_rpm = atoi(str);
        UART_TransmitString(UART_PORT, "\n\rTarget RPM: ");
        UART_TransmitInt(UART_PORT, target_rpm, 3);
        UART_TransmitString(UART_PORT, "\n\r");
        n = 0;
    } else {
        UART_TransmitChar(UART_PORT, c);
    }
}

/* -------------------------------------------------------------------------------------------------------------
 * Main Program Code
 *
 * Starts initialization of peripherals
 * Blinks green LED (PC9) in loop as heartbeat
 * -------------------------------------------------------------------------------------------------------------
 */
volatile uint32_t encoder_count = 0;

int main(void) {
    target_rpm = 0;
    debouncer = 0;                          // Initialize global variables
    HAL_Init();                             // Initialize HAL internals
    LED_init();                             // Initialize LED's
    button_init();                          // Initialize button
    motor_init();                           // Initialize motor code

    uint8_t uart = UART_PORT;
    uint8_t uart_gpio = 2;
    uint8_t uart_tx = 4; // PC4
    uint8_t uart_rx = 5; // PC5
    uint32_t baud = 115200;
    uint8_t interrupt_priority = 1;
    UART_Init(uart, uart_gpio, uart_tx, uart_rx);
    UART_SetBaudRate(uart, baud);
    UART_Enable(uart, interrupt_priority);
    UART_TransmitString(uart, "UART Works!\n\r");
    uart_rx_cb = &uart_receive;

    /*RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIO_InitTypeDef initStr = {GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6,
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOA, &initStr);
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);*/

    while (1) {
        GPIOC->ODR ^= GPIO_ODR_9;           // Toggle green LED (heartbeat)
        encoder_count = TIM2->CNT;
        HAL_Delay(128);               // Delay 1/8 second
    }
}

// ----------------------------------------------------------------------------
