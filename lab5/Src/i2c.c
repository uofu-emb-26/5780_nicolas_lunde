#include "i2c.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f072xb.h"

int i2c_nack;

void I2C_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; //Enable I2C2 clock
    I2C2->CR1 |= I2C_CR1_TXIE; // Enable TXIS interrupt
    I2C2->CR1 |= I2C_CR1_PE; // Enable I2C2
}

void I2C_SetTiming(void)
{
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
}

void I2C_Transmit(uint8_t saddr, uint8_t *data, uint8_t nbytes, int stop)
{
    // Clear bits
    I2C2->CR2 &= ~I2C_CR2_NBYTES;

    // Writing operation
    // The LSB of SADD toggles 7-bit addressing (0) or 10-bit (1)
    // Put the slave address in SADD[7:1]
    I2C2->CR2 |= (saddr << (I2C_CR2_SADD_Pos + 1)); // Slave address 
    I2C2->CR2 |= (nbytes << I2C_CR2_NBYTES_Pos); // Transfer nbytes byte
    I2C2->CR2 &= ~I2C_CR2_RD_WRN; // Write transfer
    I2C2->CR2 |= I2C_CR2_START; // Start transfer generation
 
    while (nbytes > 0)
    {
        // Wait until Transmit Register is empty or Slave Not-Acknowledged
        while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))){}
    
        // Check if slave acknowledged
        if (I2C2->ISR & I2C_ISR_NACKF)
            i2c_nack;

        I2C2->TXDR = *data; 
        nbytes--;
        data++;
    }

    // Wait until transfer is complete
    while (!(I2C2->ISR & I2C_ISR_TC)){
    }

    if (stop)
        I2C2->CR2 |= I2C_CR2_STOP;
}

void I2C_Receive(uint8_t saddr, uint8_t *data, int8_t nbytes, int stop)
{
    // Clear bits
    I2C2->CR2 &= ~I2C_CR2_NBYTES;

    // Reading operation
    I2C2->CR2 |= (saddr << (I2C_CR2_SADD_Pos + 1)); // Slave address 
    I2C2->CR2 |= (nbytes << I2C_CR2_NBYTES_Pos); // Transfer 1 byte
    I2C2->CR2 |= I2C_CR2_RD_WRN; // Read transfer
    I2C2->CR2 |= I2C_CR2_START; // Start transfer generation

    while (nbytes > 0)
    {
        // Wait until Receive Register is not empty or Slave Not-Acknowledged
        while (!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF))){}
        
        // Check if slave acknowledged
        if (I2C2->ISR & I2C_ISR_NACKF)
            i2c_nack;
        
        *data = I2C2->RXDR;
        nbytes--;
        data++;
    }

    // Wait until transfer is complete
    while (!(I2C2->ISR & I2C_ISR_TC)){}

    if (stop)
        I2C2->CR2 |= I2C_CR2_STOP;
}