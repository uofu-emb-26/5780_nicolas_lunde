#include "uart.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f072xb.h"

USART_TypeDef* PortToStruct(uint8_t port);
char BitsToHex(uint8_t bits);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_4_IRQHandler(void);

char uart1_rx_data;
int uart1_rx_flag;

char uart2_rx_data;
int uart2_rx_flag;

char uart3_rx_data;
int uart3_rx_flag;

char uart4_rx_data;
int uart4_rx_flag;

/**
 * GPIO Port: 0 = A, 1 = B, 2 = C
 * 
 * USART1: TX(PA9, PB6),            RX(PA10, PB7)
 * USART2: TX(PA14, PA2),           RX(PA15, PA3)
 * USART3: TX(PC10, PC4, PB10),     RX(PC11, PC5, PB11)
 * USART4: TX(PC10, PA0),           RX(PC11, PA1)

 * Alternate function numbers
 * Ex. AF1 is 0001, AF4 is 0100
 * USART1   -   AF1: PA9, PA10, PB6, PB7
 * USART2   -   AF1: PA2, PA3, PA14, PA15
 * USART3   -   AF1: PC4, PC5, PC10, PC11
 *          -   AF4: PB10, PB11
 * USART4   -   AF1: PC10, PC11
 *          -   AF4: PA0, PA1
*/
void UART_Init(uint8_t port, uint8_t gpio_port, uint16_t tx_pin, uint16_t rx_pin)
{
    // Enable GPIO RCC
    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN << gpio_port); 

    GPIO_InitTypeDef initStr = {(0x1 << tx_pin) | (0x1 << rx_pin),
                                GPIO_MODE_AF_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};

    // Init GPIO Pins as Alternate Function
    GPIO_TypeDef *gpio = (GPIO_TypeDef*) (GPIOA_BASE + (0x00000400UL * gpio_port));
    HAL_GPIO_Init(gpio, &initStr);
 
    // Clear alternate functions
    int afr_high = (tx_pin > 7);
    int afr_pin = (afr_high) ? tx_pin - 8 : tx_pin; 
    gpio->AFR[afr_high] &= ~((afr_pin * 4) | (afr_pin * 4)); 

    uint8_t af = ((gpio_port == 1 && tx_pin == 10 && rx_pin == 11) ||
        (gpio_port == 0 && tx_pin == 0 && rx_pin == 1)) ?
        0x44 : 0x11;

    // Set alternate functions 
    gpio->AFR[afr_high] |= (af << (afr_pin * 4));

    // Enable RCC for USART
    if (port == 1)
        RCC->APB2ENR |= (RCC_APB2ENR_USART1EN); 
    else 
        RCC->APB1ENR |= (RCC_APB1ENR_USART2EN << (port - 2));
}

void UART_SetBaudRate(uint8_t port, uint32_t baud)
{
    USART_TypeDef* usart = PortToStruct(port);
    /**
     * Baud rate = sys_clk_freq / BRR
     * BRR = sys_clk_freq / baud
     */
    uint32_t sys_clk_freq = HAL_RCC_GetHCLKFreq();
    usart->BRR = sys_clk_freq / 115200;
    usart->CR1 = USART_CR1_TE | USART_CR1_RE; // Enable transmitter and receiver
    usart->CR1 |= USART_CR1_RXNEIE; // Enable reciever not empty interrupt
} 

void UART_Enable(uint8_t port, uint8_t interrupt_priority)
{
    USART_TypeDef* usart = PortToStruct(port);
    usart->CR1 |= USART_CR1_UE; // Enable USART port 

    if (port == 4) port = 3;
    port--; // Ports start at 1. Sub to 0
    NVIC_EnableIRQ(USART1_IRQn + port); 
    NVIC_SetPriority(USART1_IRQn + port, interrupt_priority); 
}

void UART_TransmitString(uint8_t port, char* msg)
{
    USART_TypeDef* usart = PortToStruct(port);
    for (char* c = msg; *c != '\0'; c++)
    {
        UART_TransmitChar(port, *c);
    }
}

void UART_TransmitChar(uint8_t port, char c)
{
    USART_TypeDef* usart = PortToStruct(port);
    while(!(usart->ISR & USART_ISR_TXE))
    {
    }
    usart->TDR = c;
}

void UART_TransmitHex(uint8_t port, size_t num, size_t size)
{
    USART_TypeDef* usart = PortToStruct(port);
    UART_TransmitString(port, "0x");

    uint8_t *ptr = (uint8_t *) &num + size - 1;
    uint8_t upper, lower;

    for (int i = 0; i < size; i++)
    {
        upper = (*ptr & 0xF0) >> 4;
        lower = *ptr & 0x0F;
        UART_TransmitChar(port, BitsToHex(upper));
        UART_TransmitChar(port, BitsToHex(lower));
        ptr--; // Go backwards because little endian
    }
    UART_TransmitChar(port, '\0');
}

void USART1_IRQHandler(void)
{
    uart1_rx_data = USART1->RDR;
    uart1_rx_flag = 1;
}

void USART2_IRQHandler(void)
{
    uart2_rx_data = USART3->RDR;
    uart2_rx_flag = 1;
}

void USART3_4_IRQHandler(void)
{
    if (USART3->ISR & USART_ISR_RXNE)
    {
        uart3_rx_data = USART3->RDR;
        uart3_rx_flag = 1;
    }

    if (USART4->ISR & USART_ISR_RXNE)
    {
        uart4_rx_data = USART4->RDR;
        uart4_rx_flag = 1;
    }
}

USART_TypeDef* PortToStruct(uint8_t port)
{
    switch (port)
    {
    case 1:
        return USART1;
    case 2:
        return USART2;
    case 3:
        return USART3;
    case 4:
        return USART4;
    }
}

char BitsToHex(uint8_t bits)
{
    if (bits > 15) bits = 16;
    static char lookup[17] = 
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', 
        '9', 'A', 'B', 'C', 'D', 'E', 'F', '?'
    };
    return lookup[bits];
}