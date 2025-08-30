#ifndef GAME_FSM_H
#define GAME_FSM_H

typedef enum {
    eInitGame,
    eDificultSelect,
    ePersonaSelect,
    eBattleInit,
    ePlayerTurn,
    eEndGame
} EGameStates;

#endif //GAME_FSM_H
