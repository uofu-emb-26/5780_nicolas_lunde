#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

extern int i2c_nack;

void I2C_Init(void);
void I2C_SetTiming(void);
void I2C_Transmit(uint8_t saddr, uint8_t *data, uint8_t nbytes, int stop);
void I2C_Receive(uint8_t saddr, uint8_t *data, int8_t nbytes, int stop);

#endif