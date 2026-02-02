#include "main.h"
#include <stm32f0xx_hal.h>
#include "hal_gpio.h"
#include "hal_rcc.h"
#include "assert.h"

void SystemClock_Config(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    HAL_Init();           // Reset of all peripherals, init the Flash and Systick
    SystemClock_Config(); // Configure the system clock

    /* This example uses HAL library calls to control
     * the GPIOC peripheral. You'll be redoing this code
     * with hardware register access. */

    HAL_RCC_GPIOC_CLK_Enable(); // Enable the GPIOC clock in the RCC

    // Set up a configuration struct to pass to the initialization function
    GPIO_InitTypeDef initStr = {GPIO_PIN_6 | GPIO_PIN_7,
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    My_HAL_GPIO_Init(GPIOC, &initStr); // Initialize pins PC8 and PC9
    assert(GPIOC->MODER == 0x05000); // Test for output mode
    assert(GPIOC->OSPEEDR == 0x0 || GPIOC->OSPEEDR == 0xA000); // Test for low speed
    assert(GPIOC->PUPDR == 0x0); // Test for no pull-up or pull-down

    My_HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET); // Start PC6 high
    assert(GPIOC->ODR == 0x40); // Test for PC6 = 1, PC7 = 0

    // Configure USER button pin PA0
    GPIO_InitTypeDef initBtn = {GPIO_PIN_0,
                                GPIO_MODE_INPUT,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_PULLDOWN};
    My_HAL_GPIO_Init(GPIOA, &initBtn);
    GPIO_PinState prev_state = GPIO_PIN_RESET;
    while (1)
    {
        HAL_Delay(80); // Delay 200ms

        GPIO_PinState curr_state = My_HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        if (prev_state != GPIO_PIN_SET && curr_state == GPIO_PIN_SET)
        { 
            // Toggle the output state of both PC6 and PC7
            My_HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_7);
        }
        prev_state = curr_state;
        
        // Test for PC6 = 1, PC7 = 0 xor PC6 = 0, PC7 = 1
        assert(GPIOC->ODR == 0x40 ^ GPIOC->ODR == 0x80);
    }
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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
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
