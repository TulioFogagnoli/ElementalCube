#ifndef __TCA9548A_H__
#define __TCA9548A_H__

#include "stm32f4xx_hal.h"

#define MUX_ADDR (0x70 << 1) // Endereço I2C do TCA9548A

HAL_StatusTypeDef TCA9548A_SelectChannel(I2C_HandleTypeDef *hi2c, uint8_t channel);

#endif // __TCA9548A_H__