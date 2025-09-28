#include "TCA9548A.h"

HAL_StatusTypeDef TCA9548A_SelectChannel(I2C_HandleTypeDef *hi2c, uint8_t channel)
{
    if (channel > 7) return HAL_ERROR; // O multiplexador tem 8 canais (0-7)

    uint8_t buffer = 1 << channel;
    return HAL_I2C_Master_Transmit(hi2c, MUX_ADDR, &buffer, 1, HAL_MAX_DELAY);
}