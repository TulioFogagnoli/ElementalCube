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

bool TCS3472_Init(I2C_HandleTypeDef *hi2c, uint8_t channel) {
    uint8_t reg_data;

    // Seleciona o canal do sensor no multiplexador
    if (TCA9548A_SelectChannel(hi2c, channel) != HAL_OK) {
        return false; // Falha ao selecionar o canal
    }
    HAL_Delay(1); // Pequeno delay para estabilização

    // 1. Verifica se o sensor está a responder lendo o seu ID
    if (HAL_I2C_Mem_Read(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ID), 1, &reg_data, 1, HAL_MAX_DELAY) != HAL_OK) {
        return false; // Falha na comunicação I2C
    }
    if (reg_data != 0x44 && reg_data != 0x4D) {
        return false; // ID do sensor incorreto
    }

    // 2. Configura o tempo de integração do sensor
    reg_data = 0xEB; // 70ms
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ATIME), 1, &reg_data, 1, HAL_MAX_DELAY);

    // 3. Configura o ganho do sensor
    reg_data = 0x00; // Ganho 1x
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_CONTROL), 1, &reg_data, 1, HAL_MAX_DELAY);

    // 4. Ativa o oscilador interno e o conversor ADC de cor
    reg_data = TCS3472_ENABLE_PON;
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ENABLE), 1, &reg_data, 1, HAL_MAX_DELAY);
    HAL_Delay(3); // Espera o oscilador estabilizar
    reg_data |= TCS3472_ENABLE_AEN;
    HAL_I2C_Mem_Write(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_ENABLE), 1, &reg_data, 1, HAL_MAX_DELAY);

    return true; // Sucesso
}


void TCS3472_ReadData(I2C_HandleTypeDef *hi2c, uint8_t channel, TCS3472_Data* color_data) {
    uint8_t buffer[8];

    // Tenta selecionar o canal do sensor no multiplexador
    if (TCA9548A_SelectChannel(hi2c, channel) != HAL_OK) {
        // Se falhar, define um padrão de erro e retorna
        color_data->clear = 1111;
        color_data->red   = 1111;
        color_data->green = 1111;
        color_data->blue  = 1111;
        return;
    }
    HAL_Delay(1);

    // Lê os 8 bytes de dados de cor (Clear, Red, Green, Blue - 2 bytes cada)
    // VERIFICA O RETORNO DA FUNÇÃO DE LEITURA
    if (HAL_I2C_Mem_Read(hi2c, TCS3472_ADDRESS, (TCS3472_COMMAND_BIT | TCS3472_REG_CDATAL), 1, buffer, 8, HAL_MAX_DELAY) != HAL_OK)
    {
        // Se a leitura I2C falhar, define um padrão de erro diferente
        color_data->clear = 2222;
        color_data->red   = 2222;
        color_data->green = 2222;
        color_data->blue  = 2222;
        return;
    }   

    // Se a leitura for bem-sucedida, processa os dados
    color_data->clear = (buffer[1] << 8) | buffer[0];
    color_data->red   = (buffer[3] << 8) | buffer[2];
    color_data->green = (buffer[5] << 8) | buffer[4];
    color_data->blue  = (buffer[7] << 8) | buffer[6];
}

EColor TCS3472_DetectColor(TCS3472_Data data) {
    uint16_t red = data.red;
    uint16_t green = data.green;
    uint16_t blue = data.blue;
    uint16_t clear = data.clear;

    // REGRA 1: Detetar ausência de cor (Preto ou Vazio)
    // Se a intensidade da luz (clear) for muito baixa, não há cor.
    // Baseado nos seus dados, um limiar de 150 parece seguro.
    if (clear < 150) {
        return eBlack;
    }

    // REGRA 2: Detetar Amarelo
    // Baseado nos seus dados (R=518, G=443, B=152), R e G são altos e B é baixo.
    // Verificamos se R e G são pelo menos 2 vezes maiores que B.
    if (red > blue * 2 && green > blue * 2) {
        return eYellow;
    }

    // REGRA 3: Detetar Vermelho
    // Baseado nos seus dados (R=277, G=97, B=74), R é muito maior que G e B.
    // Verificamos se R é pelo menos 2 vezes maior que G e B.
    if (red > green * 2 && red > blue * 2) {
        return eRed;
    }
    
    // REGRA 4: Detetar Verde
    // Baseado nos seus dados (G=148, R=69, B=68), G é muito maior que R e B.
    // Verificamos se G é pelo menos 1.5 vezes maior que R e B.
    if (green > red * 1.5 && green > blue * 1.5) {
        return eGreen;
    }

    // REGRA 5: Detetar Azul (no seu caso, um Ciano/Azul-Esverdeado)
    // Baseado nos seus dados (G=153, B=136, R=67), G e B são altos e R é baixo.
    // Verificamos se G e B são maiores que R.
    if (green > red && blue > red) {
        return eBlue;
    }

    // REGRA 6: Detetar Branco
    // Baseado nos seus dados, todos os valores são muito altos.
    // Usamos um limiar alto no clear para identificar o branco.
    if (clear > 2000) {
        return eWhite;
    }       

    // Se nenhuma cor dominar claramente, pode ser branco
    return eBlack;
}


/*
COR      |   R  |  G   |  B  |  C
---------/------/------/-----/----
VAZIO    |  09  | 07   | 05  | 22
VERMELHO | 277  | 97   | 74  | 41
AZUL     |  67  | 153  | 136 | 290
AMARELO  | 518  | 443  | 152 | 1166
VERDE    |  69  | 148  | 68  | 287
BRANCO   | 1007 | 1253 | 773 | 3148
PRETO    |  42  | 45   | 33  | 122


*/