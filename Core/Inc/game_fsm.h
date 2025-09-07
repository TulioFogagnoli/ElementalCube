#ifndef GAME_FSM_H
#define GAME_FSM_H

#include "game_types.h"

typedef enum {
    eInitGame,
    eDificultSelect,
    ePersonaSelect,
    eBattleInit,
    ePlayerTurn,
    eEndGame
} EGameStates;

void vInitBattle(EWizard ePlayer);
#endif //GAME_FSM_H
