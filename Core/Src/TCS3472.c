#include "TCS3472.h"

// Registos do sensor
#define TCS3472_COMMAND_BIT      0x80
#define TCS3472_REG_ENABLE       0x00
#define TCS3472_REG_ATIME        0x01
#define TCS3472_REG_CONTROL      0x0F
#define TCS3472_REG_ID           0x12
#define TCS3472_REG_CDATAL       0x14

// Configurações de inicialização
#define TCS3472_ENABLE_PON       0x01 // Power ON
#define TCS3472_ENABLE_AEN       0x02 // RGBC ADC Enable

/**
 * @brief Inicializa o sensor TCS3472.
 * @param hi2c: Ponteiro para a handle do I2C a ser usado.
 * @return true se a inicialização foi bem-sucedida, false caso contrário.
 */
bool TCS3472_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t reg_data;

    // 1. Verifica se o sensor está a responder lendo o seu ID
    // O ID do TCS3472 deve ser 0x44 ou 0x4D
    HAL_I2C_Mem_Read(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ID), 1, &reg_data, 1, HAL_MAX_DELAY);
    if (reg_data != 0x44 && reg_data != 0x4D) {
        return false; // Falha na comunicação
    }

    // 2. Configura o tempo de integração do sensor (afeta a sensibilidade)
    // 0xEB = 70ms. Valores mais altos = mais sensível à luz fraca.
    reg_data = 0xEB;
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ATIME), 1, &reg_data, 1, HAL_MAX_DELAY);

    // 3. Configura o ganho do sensor (1x, 4x, 16x, 60x)
    // 0x00 = Ganho 1x
    reg_data = 0x00;
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_CONTROL), 1, &reg_data, 1, HAL_MAX_DELAY);

    // 4. Ativa o oscilador interno e o conversor ADC de cor
    reg_data = TCS3472_ENABLE_PON;
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ENABLE), 1, &reg_data, 1, HAL_MAX_DELAY);
    HAL_Delay(3); // Espera o oscilador estabilizar
    reg_data |= TCS3472_ENABLE_AEN;
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ENABLE), 1, &reg_data, 1, HAL_MAX_DELAY);

    return true; // Sucesso
}

/**
 * @brief Lê os valores de Vermelho, Verde, Azul e Clear do sensor.
 * @param hi2c: Ponteiro para a handle do I2C a ser usado.
 * @param color_data: Ponteiro para a estrutura onde os dados serão armazenados.
 */
void TCS3472_ReadData(I2C_HandleTypeDef *hi2c, TCS3472_Data* color_data) {
    uint8_t buffer[8];

    // Lê os 8 bytes de dados de cor (Clear, Red, Green, Blue - 2 bytes cada)
    // O sensor auto-incrementa o endereço do registo, por isso podemos ler tudo de uma vez
    HAL_I2C_Mem_Read(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_CDATAL), 1, buffer, 8, HAL_MAX_DELAY);

    color_data->clear = (buffer[1] << 8) | buffer[0];
    color_data->red   = (buffer[3] << 8) | buffer[2];
    color_data->green = (buffer[5] << 8) | buffer[4];
    color_data->blue  = (buffer[7] << 8) | buffer[6];
}