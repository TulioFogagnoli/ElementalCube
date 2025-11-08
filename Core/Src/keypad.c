#include "keypad.h"
#include "main.h" // Inclui as definições da HAL E os defines BTN_... e de caracteres

/*
==================================================================================
 PASSO 1: MAPEAMENTO FÍSICO DOS PINOS
 
 Não é mais necessário! Os defines (BTN_UP_PORT, BTN_UP_PIN, etc.)
 agora estão (corretamente) no seu arquivo "main.h".

==================================================================================
*/


/*
==================================================================================
 PASSO 2: MAPEAMENTO DOS CARACTERES DO JOGO (A CORREÇÃO ESTÁ AQUI!)
 
 Lógica do main.c:
 - '8' (UP_KEY) move a seleção para BAIXO (+1)
 - '2' (DOWN_KEY) move a seleção para CIMA (-1)

 Nossa lógica (para corrigir a inversão):
 - Botão CIMA (Verde, PD6) deve enviar '2' (DOWN_KEY) para mover a seleção para CIMA.
 - Botão BAIXO (Amarelo, PD7) deve enviar '8' (UP_KEY) para mover a seleção para BAIXO.
==================================================================================
*/
#define BUTTON_CONFIRM_CHAR  CONFIRM_KEY // Usa o define de main.h ('*')
#define BUTTON_BACK_CHAR     BACK_KEY    // Usa o define de main.h ('#')
#define BUTTON_UP_CHAR       DOWN_KEY    // Verde (Cima) envia '2'
#define BUTTON_DOWN_CHAR     UP_KEY      // Amarelo (Baixo) envia '8'


/*
==================================================================================
 PASSO 3: LÓGICA DE DETECÇÃO DE BORDA (NÃO-BLOQUEANTE)
 (Sem alterações aqui)
==================================================================================
*/
static uint8_t btn_up_state = 1;      // Verde
static uint8_t btn_down_state = 1;    // Amarelo
static uint8_t btn_confirm_state = 1; // Vermelho
static uint8_t btn_back_state = 1;    // Preto


/**
 * @brief Função não-bloqueante que detecta a borda de descida (press) 
 * de botões momentâneos.
 * Esta função é chamada a cada 50ms pela StartInputHalTask.
 */
char KEYPAD_Scan(void) {
    
    // ----- Lógica do Botão CIMA (Verde - PD6) -----
    // Usamos BTN_UP_PORT e BTN_UP_PIN direto de main.h
    uint8_t current_up = HAL_GPIO_ReadPin(BTN_UP_PORT, BTN_UP_PIN);
    if (current_up == GPIO_PIN_RESET && btn_up_state == 1) {
        btn_up_state = 0;
        return BUTTON_UP_CHAR; // Retorna '2'
    } 
    else if (current_up == GPIO_PIN_SET && btn_up_state == 0) {
        btn_up_state = 1;
    }

    // ----- Lógica do Botão BAIXO (Amarelo - PD7) -----
    // Usamos BTN_DOWN_PORT e BTN_DOWN_PIN direto de main.h
    uint8_t current_down = HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN);
    if (current_down == GPIO_PIN_RESET && btn_down_state == 1) {
        btn_down_state = 0;
        return BUTTON_DOWN_CHAR; // Retorna '8'
    } 
    else if (current_down == GPIO_PIN_SET && btn_down_state == 0) {
        btn_down_state = 1;
    }

    // ----- Lógica do Botão CONFIRMAR (Vermelho - PD4) -----
    // Usamos BTN_CONFIRM_PORT e BTN_CONFIRM_PIN direto de main.h
    uint8_t current_confirm = HAL_GPIO_ReadPin(BTN_CONFIRM_PORT, BTN_CONFIRM_PIN);
    if (current_confirm == GPIO_PIN_RESET && btn_confirm_state == 1) {
        btn_confirm_state = 0;
        return BUTTON_CONFIRM_CHAR; // Retorna '*'
    } 
    else if (current_confirm == GPIO_PIN_SET && btn_confirm_state == 0) {
        btn_confirm_state = 1;
    }

    // ----- Lógica do Botão VOLTAR (Preto - PD5) -----
    // Usamos BTN_BACK_PORT e BTN_BACK_PIN direto de main.h
    uint8_t current_back = HAL_GPIO_ReadPin(BTN_BACK_PORT, BTN_BACK_PIN);
    if (current_back == GPIO_PIN_RESET && btn_back_state == 1) {
        btn_back_state = 0;
        return BUTTON_BACK_CHAR; // Retorna '#'
    } 
    else if (current_back == GPIO_PIN_SET && btn_back_state == 0) {
        btn_back_state = 1;
    }

    return '\0'; // Retorna nulo se nenhuma *nova* tecla for pressionada
}