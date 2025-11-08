#ifndef GAME_FSM_H
#define GAME_FSM_H

#include "game_types.h" 

#define ATTACKS_NUMBERS 4
#define BASE_ATTACK_DAMAGE 10
#define SUPER_EFFECTIVE_MODIFIER 1.5f   // Dano 1.5x
#define NOT_EFFECTIVE_MODIFIER 0.5f // Metade do dano


typedef enum
{
  eOutcome_Neutral,
  eOutcome_SuperEffective,
  eOutcome_NotEffective
} EAttackOutcome;

void vInitBattle(EWizard *eUserPlayer, EWizard *eCpuPlayer);
EAttackOutcome eGetAttackOutcome(EColor eAttackerAttack, EColor eDefenderAttack);

#endif //GAME_FSM_H