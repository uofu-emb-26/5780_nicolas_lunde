#include "main.h"
#include "stm32f0xx_hal.h"
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

    // UART setup 
    uint8_t uart_port = 3;
    uint8_t uart_gpio = 2; // GPIOC
    uint8_t tx_pin = 4; // PC4
    uint8_t rx_pin = 5; // PC5
    uint32_t baud = 115200;
    uint8_t interrupt_priority;
    UART_Init(uart_port, uart_gpio, tx_pin, rx_pin);
    UART_SetBaudRate(uart_port, baud);
    UART_Enable(uart_port, interrupt_priority);
    UART_TransmitString(uart_port, "UART Works!\r");

    // Enable RCC for GPIOB and GPIOC
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN; 

    GPIO_InitTypeDef init_sda_scl = {GPIO_PIN_11 | GPIO_PIN_13,
                                GPIO_MODE_AF_OD,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};

    HAL_GPIO_Init(GPIOB, &init_sda_scl);
    GPIOB->AFR[1] |= (0x1 << GPIO_AFRH_AFSEL11_Pos); // Set PB11 AF to I2C2_SDA
    GPIOB->AFR[1] |= (0x5 << GPIO_AFRH_AFSEL13_Pos); // Set PB11 AF to I2C2_SCL

    GPIO_InitTypeDef init_pb14 = {GPIO_PIN_14, 
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOB, &init_pb14);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    
    GPIO_InitTypeDef init_pc0 = {GPIO_PIN_0, 
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &init_pc0);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);

    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; //Enable I2C2 clock

    /**
     * Set to 100 kHz
     * fI2CCLK = 8 MHz
     * tI2CCLK = 1 / fI2CCLK = 125 ns
     * tPRESC = (PRESC+1) * tI2CCLK = (1+1) * 125 ns = 250 ns
     * tSCLL = (SCLL+1) * tPRESC = 20 * 250 ns = 50 us
     * tSCLH = (SCLH+1) * tPRESC = 16 * 250 ns = 4 us
     * tSDADEL = SDADEL * tPRESC = 2 * 250 ns = 500 ns
     * tSCLDEL = SCLDEL * tPRESC = 5 * 250 ns = 1250 ns
    */ 
    I2C2->TIMINGR |= (19 << I2C_TIMINGR_SCLL_Pos); // SCL low period
    I2C2->TIMINGR |= (15 << I2C_TIMINGR_SCLH_Pos); // SCL high period
    I2C2->TIMINGR |= (2 << I2C_TIMINGR_SDADEL_Pos); // Data hold time 
    I2C2->TIMINGR |= (4 << I2C_TIMINGR_SCLDEL_Pos); // Data setup time 
    I2C2->TIMINGR |= (1 << I2C_TIMINGR_PRESC_Pos); // Timing prescaler

    // Writing operation
    I2C2->CR1 |= I2C_CR1_PE_Pos; // Enable I2C2
    I2C2->CR2 |= (0x6B << I2C_CR2_SADD_Pos); // Slave address for L3GD20 gyro sensor
    I2C2->CR2 |= (0x0 << I2C_CR2_RD_WRN_Pos); // Write transfer
    I2C2->CR2 |= (0x1 << I2C_CR2_START_Pos); // Start transfer generation

    // Wait until Transmit Register is empty or Slave Not-Acknowledged
    UART_TransmitString(uart_port, "Stops here!\r");
    while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))){}

    if (I2C2->ISR & I2C_ISR_NACKF)
    {
        return 0;
    }

    I2C2->TXDR = 0x0F; // Address of WHO_AM_I register in L3GD20
    // Wait until transfer is complete
    while (!(I2C2->ISR & I2C_ISR_TC)){}

    // Reading operation
    I2C2->CR1 |= I2C_CR1_PE_Pos; // Enable I2C2
    I2C2->CR2 |= (0x6B << I2C_CR2_SADD_Pos); // Slave address for L3GD20 gyro sensor
    I2C2->CR2 |= (0x1 << I2C_CR2_RD_WRN_Pos); // Read transfer
    I2C2->CR2 |= (0x1 << I2C_CR2_START_Pos); // Start transfer generation

    // Wait until Receive Register is not empty or Slave Not-Acknowledged
    while (!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF))){}
    
    if (I2C2->ISR & I2C_ISR_NACKF)
    {
        return 0;
    }

    // Wait until transfer is complete
    while (!(I2C2->ISR & I2C_ISR_TC)){}
    
    uint8_t data = I2C2->RXDR;
    I2C2->CR2 |= I2C_CR2_STOP_Pos;
    UART_TransmitInt(uart_port, data, 4); 

    // Flashing LED
    GPIO_InitTypeDef initStr = {GPIO_PIN_6, 
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &initStr); 

    while (1)
    {
        HAL_Delay(1000);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
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