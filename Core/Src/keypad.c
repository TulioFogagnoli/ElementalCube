#include "keypad.h"
#include "main.h" // Onde os defines dos pinos (User Labels) estão

// Mapeamento do teclado 4x4
const char KEYPAD_MAP[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Estruturas para facilitar o acesso aos pinos
typedef struct {
    GPIO_TypeDef* PORT;
    uint16_t PIN;
} Keypad_Pin_t;

// Pinos das colunas (saídas)
Keypad_Pin_t C_PINS[4] = {{C4_GPIO_Port, C4_Pin}, {C3_GPIO_Port, C3_Pin}, {C2_GPIO_Port, C2_Pin}, {C1_GPIO_Port, C1_Pin}};
// Pinos das linhas (entradas)
Keypad_Pin_t R_PINS[4] = {{R2_GPIO_Port, R2_Pin}, {R1_GPIO_Port, R1_Pin}, {R3_GPIO_Port, R3_Pin}, {R4_GPIO_Port, R4_Pin}};


char KEYPAD_Scan(void) {
    // Coloca todas as colunas em nível alto
    for (int i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(C_PINS[i].PORT, C_PINS[i].PIN, GPIO_PIN_SET);
    }

    // Loop para varrer cada coluna
    for (int col = 0; col < 4; col++) {
        // Ativa a coluna atual (coloca em nível baixo)
        HAL_GPIO_WritePin(C_PINS[col].PORT, C_PINS[col].PIN, GPIO_PIN_RESET);

        // Verifica qual linha foi para nível baixo
        for (int row = 0; row < 4; row++) {
            if (HAL_GPIO_ReadPin(R_PINS[row].PORT, R_PINS[row].PIN) == GPIO_PIN_RESET) {
                // Botão pressionado!
                
                // --- Debounce e espera soltar a tecla ---
                HAL_Delay(50); // Simples debounce por atraso

                // Espera o usuário soltar a tecla para não ler a mesma tecla várias vezes
                while(HAL_GPIO_ReadPin(R_PINS[row].PORT, R_PINS[row].PIN) == GPIO_PIN_RESET);
                
                // Restaura a coluna para nível alto antes de retornar
                HAL_GPIO_WritePin(C_PINS[col].PORT, C_PINS[col].PIN, GPIO_PIN_SET);

                return KEYPAD_MAP[row][col];
            }
        }

        // Desativa a coluna atual antes de ir para a próxima
        HAL_GPIO_WritePin(C_PINS[col].PORT, C_PINS[col].PIN, GPIO_PIN_SET);
    }

    return '\0'; // Retorna nulo se nenhuma tecla for pressionada
}