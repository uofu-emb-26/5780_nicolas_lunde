#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "led.h"
#include "adc.h"
#include "uart.h"

void SystemClock_Config(void);

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

    uint8_t uart = 3;
    uint8_t uart_gpio = 2;
    uint8_t uart_tx = 4; // PC4
    uint8_t uart_rx = 5; // PC5
    uint32_t baud = 115200;
    uint8_t interrupt_priority = 1;
    UART_Init(uart, uart_gpio, uart_tx, uart_rx);
    UART_SetBaudRate(uart, baud);
    UART_Enable(uart, interrupt_priority);
    UART_TransmitString(uart, "UART Works!\r\n");

    InitLEDs(LED_RED | LED_GREEN | LED_BLUE | LED_ORANGE);
    uint8_t channel = 0;
    ADC_InitPin(channel);
    ADC_Init(channel);

    uint8_t data = 0;
    uint8_t orange_th = 64;
    uint8_t red_th = 128;
    uint8_t green_th = 192;
    uint8_t blue_th = 255;

    while (1)
    {
        HAL_GPIO_WritePin(GPIOC, LED_ORANGE, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, LED_RED, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, LED_GREEN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, LED_BLUE, GPIO_PIN_RESET);

        if (ADC1->ISR & ADC_ISR_OVR) {
            ADC1->ISR |= ADC_ISR_OVR; 
        }

        while (!(ADC1->ISR & ADC_ISR_EOC)) {} // Wait until conversion ends
        data = ADC1->DR; // Right aligned 8 bits

        UART_TransmitHex(3, data, 1);
        UART_TransmitString(3, "\r\n");

        if (data >= orange_th)
            HAL_GPIO_WritePin(GPIOC, LED_ORANGE, GPIO_PIN_SET);
        else if (data >= red_th)
            HAL_GPIO_WritePin(GPIOC, LED_RED, GPIO_PIN_SET);
        else if (data >= green_th)
            HAL_GPIO_WritePin(GPIOC, LED_GREEN, GPIO_PIN_SET);
        else if (data >= blue_th) 
            HAL_GPIO_WritePin(GPIOC, LED_BLUE, GPIO_PIN_SET);
    }
    return -1;
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
