#ifndef GAME_FSM_H
#define GAME_FSM_H

#include "game_types.h"
#define ATTACKS_NUMBERS 4
#define BASE_ATTACK_DAMAGE 10
#define SUPER_EFFECTIVE_MODIFIER 1.5f   // Dano dobrado
#define NOT_EFFECTIVE_MODIFIER 0.5f // Metade do dano

typedef enum {
    eInitGame,
    eDificultSelect,
    ePersonaSelect,
    eBattleInit,
    eBattleResolution,
    ePlayerTurn,
    eEndGame
} EGameStates;

typedef enum
{
  eOutcome_Neutral,
  eOutcome_SuperEffective,
  eOutcome_NotEffective
} EAttackOutcome;

typedef struct {
    int damageToUser;
    int damageToCpu;
    EAttackOutcome userOutcome; // Útil se quiser desenhar "Critical" ou "Block" na tela
    EAttackOutcome cpuOutcome;
} BattleRoundResult;

extern BattleRoundResult battleResults[ATTACKS_NUMBERS];

void vInitBattle(EWizard *eUserPlayer, EWizard *eCpuPlayer);
EAttackOutcome eGetAttackOutcome(EColor eAttackerAttack, EColor eDefenderAttack);

#endif //GAME_FSM_H
