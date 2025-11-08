#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>


typedef enum {
    eElementalFire, 
    eElementalWater, 
    eElementalEarth, 
    eElementalAir, 
    eElementalLight, 
    eElementalShadow
} EElemental;

typedef enum {
    eRed,
    eBlue,
    eGreen,
    eYellow,
    eWhite,
    eBlack
} EColor;

typedef enum {
    eDificultEasy,
    eDificultMedium,
    eDificultHard
} EDificult;

typedef struct {
    uint8_t u8HeartPoints;
    EElemental ePersonaElemental;
    EColor eAttackSequential[4]; 
} EWizard;

typedef enum {
    eInitGame,
    eDificultSelect,
    ePersonaSelect,
    eBattleInit,
    ePlayerTurn,
    eEndGame
} EGameStates;

/**
 * @brief (Fila de Entrada) Define os tipos de eventos de entrada.
 * Por enquanto, só temos o teclado.
 */
typedef enum {
    INPUT_EVENT_KEYPAD
} InputEventType_t;

/**
 * @brief (Fila de Entrada) A mensagem que a tarefa de Input
 * enviará para a tarefa de Game.
 */
typedef struct {
    InputEventType_t eType; // Tipo do evento (ex: KEYPAD)
    char             cValue;  // Valor (ex: a tecla pressionada)
} InputEvent_t;


/**
 * @brief (Fila de Display) A "ViewModel".
 * Esta ÚNICA struct contém TUDO que a tarefa de Display
 * precisa saber para desenhar QUALQUER tela do jogo.
 * A tarefa de Game irá preencher e enviar isso.
 */
typedef struct {
    // O estado que o display deve renderizar
    EGameStates     eStateToRender;     
    
    // Dados para os menus (Dificuldade, Personagem)
    int             nSelectedOption;
    
    // Dados para as telas de batalha (Início, Turno, Fim)
    EWizard         stUserPlayer; // Envia uma CÓPIA dos dados
    EWizard         stCpuPlayer;
    
    // Flag para o display saber se o SD Card funcionou
    uint8_t         u8SdCardMounted;

} DisplayState_t;


#endif //GAME_TYPES_H