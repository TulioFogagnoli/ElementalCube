#ifndef __TCS3472_H__
#define __TCS3472_H__

#include "main.h"
#include <stdbool.h>

// Endereço I2C fixo do sensor
#define TCS3472_ADDRESS (0x29 << 1) // Usamos (0x29 << 1) porque a HAL usa endereços de 8 bits

// Estrutura para armazenar os dados de cor lidos
typedef struct {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t clear; // Nível de luz ambiente (sem filtro)
} TCS3472_Data;

// Funções do driver
bool TCS3472_Init(I2C_HandleTypeDef *hi2c);
void TCS3472_ReadData(I2C_HandleTypeDef *hi2c, TCS3472_Data* color_data);

#endif // __TCS3472_H__