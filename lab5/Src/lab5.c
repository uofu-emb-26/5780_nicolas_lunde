#include "main.h"
#include "stm32f0xx_hal.h"
#include "uart.h"
#include "i3g4250d_driver.h"

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

    // UART setup 
    uint8_t uart_port = 3;
    uint8_t uart_gpio = 2; // GPIOC
    uint8_t tx_pin = 4; // PC4
    uint8_t rx_pin = 5; // PC5
    uint32_t baud = 115200;
    uint8_t interrupt_priority = 1;
    UART_Init(uart_port, uart_gpio, tx_pin, rx_pin);
    UART_SetBaudRate(uart_port, baud);
    UART_Enable(uart_port, interrupt_priority);
    UART_TransmitString(uart_port, "UART Works!\r\n");

    // Enable RCC for GPIOB and GPIOC
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN; 
    
    I3G4_Init();
    uint8_t data = I3G4_ReadRegister(I3G4_WHO_AM_I);
    UART_TransmitString(uart_port, "WHO_AM_I: ");
    UART_TransmitHex(uart_port, data, 1); 
    UART_TransmitString(uart_port, "\r\n");

    // Enable X and Y axes and enable sleep mode
    I3G4_WriteRegister(I3G4_CTRL_REG1, 0x0B);

    // Flashing LED
    GPIO_InitTypeDef initStr = {GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, 
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &initStr); 

    int16_t x = 0 ;
    int16_t y = 0;

    while (1)
    {
        HAL_Delay(100);
        
        x = I3G4_ReadRegister(I3G4_OUT_X_H);
        x <<= 8;
        x |= I3G4_ReadRegister(I3G4_OUT_X_L);

        y = I3G4_ReadRegister(I3G4_OUT_Y_H);
        y <<= 8;
        y |= I3G4_ReadRegister(I3G4_OUT_Y_L);

        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

        if (x > 0) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
        if (x < 0) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
        if (y > 0) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        if (y < 0) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
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