#include "main.h"
#include "stm32f0xx_hal.h"
#include <assert.h>

void SystemClock_Config(void);
void TransmitChar(char c);
void TransmitString(char* msg);
void USART3_4_IRQHandler(void);

char g_rx_data;
int g_rx_flag;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();

    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN; 

    GPIO_InitTypeDef initStr = {GPIO_PIN_10 | GPIO_PIN_11,
                                GPIO_MODE_AF_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOB, &initStr);
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL10_Msk | GPIO_AFRH_AFSEL11_Msk); // Clear AFSEL 10 and 11
    GPIOB->AFR[1] |= (0x44 << GPIO_AFRH_AFSEL10_Pos); // Set AFSEL 10 and 11 to AF4
    
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN; // Enable RCC for USART

    /**
     * Baud rate = 115200 bits/second
     * Baud rate = sys_clk_freq / BRR
     * BRR = sys_clk_freq / 115200
     */
    uint32_t sys_clk_freq = HAL_RCC_GetHCLKFreq();
    USART3->BRR = sys_clk_freq / 115200;
    USART3->CR1 = USART_CR1_TE | USART_CR1_RE; // Enable transmitter and receiver
    USART3->CR1 |= USART_CR1_RXNEIE; // Enable reciever not empty interrupt

    USART3->CR1 |= USART_CR1_UE; // Enable USART3

    NVIC_EnableIRQ(USART3_4_IRQn);       // Enable USART3 in NVIC
    NVIC_SetPriority(USART3_4_IRQn, 1);  // Set high priority

    GPIO_InitTypeDef initStr1 = {GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &initStr1);

    char* error_color_msg = "\nLED color not found. Use either r, g, b, o";
    char* error_action_msg = "\nAction not found. Use either 0, 1, 2";
    uint16_t pin_num; 
    int color_picked = 0; 
    TransmitString("CMD$ ");
    while (1)
    {
        while (!g_rx_flag)
        {
        }

        TransmitChar(g_rx_data);
        if (!color_picked)
        { 
            color_picked = 1;
            switch (g_rx_data)
            {
            case 'r':
                pin_num = GPIO_PIN_6;
                break;
            case 'g':
                pin_num = GPIO_PIN_9;
                break;
            case 'b':
                pin_num = GPIO_PIN_7;
                break;
            case 'o':
                pin_num = GPIO_PIN_8;
                break;
            default:
                TransmitString(error_color_msg);
                TransmitString("\nCMD$ ");
                color_picked = 0;
                break;
            }
        }
        else
        {
            switch (g_rx_data)
            {
            case '0':
                HAL_GPIO_WritePin(GPIOC, pin_num, GPIO_PIN_RESET);
                break;
            case '1':
                HAL_GPIO_WritePin(GPIOC, pin_num, GPIO_PIN_SET);
                break;
            case '2':
                HAL_GPIO_TogglePin(GPIOC, pin_num);
                break;
            default:
                TransmitString(error_action_msg);
                break;
            }
            color_picked = 0;
            TransmitString("\nCMD$ ");
        }
        g_rx_flag = 0;
    }
    return -1;
}

void TransmitString(char* msg)
{
    for (char* c = msg; *c != '\0'; c++)
    {
        TransmitChar(*c);
        assert(USART3->TDR == *c);
    }
}

void TransmitChar(char c)
{
    while(!(USART3->ISR & USART_ISR_TXE))
    {
    }
    USART3->TDR = c;
}

void USART3_4_IRQHandler(void)
{
    g_rx_data = USART3->RDR;
    g_rx_flag = 1;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* User can add their own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add their own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
