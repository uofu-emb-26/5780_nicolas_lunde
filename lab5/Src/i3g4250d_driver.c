#include "i3g4250d_driver.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f072xb.h"
#include "i2c.h"

int i3g4_nack;

int I3G4_Init()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN; 

    GPIO_InitTypeDef init_sda_scl = {GPIO_PIN_11 | GPIO_PIN_13,
                                GPIO_MODE_AF_OD,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};

    HAL_GPIO_Init(GPIOB, &init_sda_scl);
    GPIOB->AFR[1] |= (0x1 << GPIO_AFRH_AFSEL11_Pos); // Set PB11 AF to I2C2_SDA
    GPIOB->AFR[1] |= (0x5 << GPIO_AFRH_AFSEL13_Pos); // Set PB13 AF to I2C2_SCL

    GPIO_InitTypeDef init_pb14 = {GPIO_PIN_14, 
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOB, &init_pb14);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    
    GPIO_InitTypeDef init_pb15 = {GPIO_PIN_15, 
                                GPIO_MODE_INPUT,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOB, &init_pb15);

    GPIO_InitTypeDef init_pc0 = {GPIO_PIN_0, 
                                GPIO_MODE_OUTPUT_PP,
                                GPIO_SPEED_FREQ_LOW,
                                GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &init_pc0);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);

    I2C_SetTiming();
    I2C_Init();
}

void I3G4_WriteRegister(uint8_t addr, uint8_t data)
{
    // Write the register address to modify
    // Followed by the data
    uint8_t addr_data[2] = {addr, data};
    I2C_Transmit(I3G4_ADDR, addr_data, 2, 1);
    i3g4_nack = i2c_nack;
}

uint8_t I3G4_ReadRegister(uint8_t addr)
{
    // Write the register address to read
    I2C_Transmit(I3G4_ADDR, &addr, 1, 0);

    // Restart and read register Read register
    uint8_t data;
    I2C_Receive(I3G4_ADDR, &data, 1, 1);

    i3g4_nack = i2c_nack;
    return data;
}