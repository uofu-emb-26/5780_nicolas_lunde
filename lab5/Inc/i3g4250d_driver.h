#ifndef __I3G4250D_DRIVER_H
#define __I3G4250D_DRIVER_H

// Simple driver code for I3G4250D gyro sensor that doesn't use HAL code
// Uses i2c

#include <stdint.h>

// Register Addresses
#define I3G4_ADDR 0x69
#define I3G4_WHO_AM_I 0x0F
#define I3G4_CTRL_REG1 0x20
#define I3G4_OUT_X_L 0x28
#define I3G4_OUT_X_H 0x29
#define I3G4_OUT_Y_L 0x2A
#define I3G4_OUT_Y_H 0x2B

extern int i3g4_nack;

int I3G4_Init(); 
void I3G4_WriteRegister(uint8_t addr, uint8_t data);
uint8_t I3G4_ReadRegister(uint8_t addr);

#endif