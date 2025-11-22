#include "game_fsm.h"

BattleRoundResult battleResults[ATTACKS_NUMBERS];

/*
Atacante	|Defensor	|Resultado para o Atacante
Fogo	    |Terra	    |Super Efetivo
Fogo	    |Sombra	    |Super Efetivo
Fogo	    |Água	    |Pouco Efetivo
Água	    |Fogo	    |Super Efetivo
Água	    |Ar	        |Super Efetivo
Água	    |Terra	    |Pouco Efetivo
Terra	    |Água	    |Super Efetivo
Terra	    |Luz	    |Super Efetivo
Terra	    |Fogo	    |Pouco Efetivo
Ar	        |Terra	    |Super Efetivo
Ar	        |Fogo	    |Pouco Efetivo
Luz	        |Sombra	    |Super Efetivo
Luz	        |Terra	    |Pouco Efetivo
Sombra	    |Luz	    |Super Efetivo
Sombra	    |Fogo	    |Pouco Efetivo

*/

/**
 * @brief Executa a lógica de batalha turno a turno, atualizando a vida dos jogadores.
 * @param pUserPlayer: Ponteiro para o jogador do usuário.
 * @param pCpuPlayer: Ponteiro para o jogador da CPU.
 */
void vInitBattle(EWizard* pUserPlayer, EWizard* pCpuPlayer)
{
  for (uint8_t u8Idx = 0; u8Idx < ATTACKS_NUMBERS; u8Idx++)
  {
    // 1. Identifica ataques
    EColor userAttack = pUserPlayer->eAttackSequential[u8Idx];
    EColor cpuAttack = pCpuPlayer->eAttackSequential[u8Idx];

    // 2. Calcula Dano Player -> CPU
    EAttackOutcome userOutcome = eGetAttackOutcome(userAttack, cpuAttack);
    int userDamage = BASE_ATTACK_DAMAGE;
    if (userOutcome == eOutcome_SuperEffective) userDamage *= SUPER_EFFECTIVE_MODIFIER;
    else if (userOutcome == eOutcome_NotEffective) userDamage *= NOT_EFFECTIVE_MODIFIER;

    // 3. Calcula Dano CPU -> Player
    EAttackOutcome cpuOutcome = eGetAttackOutcome(cpuAttack, userAttack);
    int cpuDamage = BASE_ATTACK_DAMAGE;
    if (cpuOutcome == eOutcome_SuperEffective) cpuDamage *= SUPER_EFFECTIVE_MODIFIER;
    else if (cpuOutcome == eOutcome_NotEffective) cpuDamage *= NOT_EFFECTIVE_MODIFIER;

    // 4. Armazena no buffer (SEM subtrair HP ainda)
    battleResults[u8Idx].damageToCpu = userDamage;
    battleResults[u8Idx].damageToUser = cpuDamage;
    battleResults[u8Idx].userOutcome = userOutcome;
    battleResults[u8Idx].cpuOutcome = cpuOutcome;
  }
}

/**
 * @brief Calcula o resultado de um ataque baseado nos elementos, incluindo Luz e Sombra.
 * @param eAttackerAttack: O elemento do atacante.
 * @param eDefenderAttack: O elemento do defensor no mesmo turno.
 * @retval EAttackOutcome: O resultado da interação.
 */
EAttackOutcome eGetAttackOutcome(EColor eAttackerAttack, EColor eDefenderAttack)
{
  switch (eAttackerAttack)
  {
    case eRed: // FOGO
      if (eDefenderAttack == eYellow) return eOutcome_SuperEffective; // Fogo > Terra
      if (eDefenderAttack == eBlack) return eOutcome_SuperEffective;  //  Fogo > Sombra
      if (eDefenderAttack == eBlue) return eOutcome_NotEffective;     // Fogo < Agua
      break;

    case eBlue: // ÁGUA
      if (eDefenderAttack == eRed) return eOutcome_SuperEffective;    // Agua > Fogo
      if (eDefenderAttack == eGreen) return eOutcome_SuperEffective;  // Agua > Ar
      if (eDefenderAttack == eYellow) return eOutcome_NotEffective;   // Agua < Terra
      break;

    case eYellow: // TERRA
      if (eDefenderAttack == eBlue) return eOutcome_SuperEffective;   // Terra > Agua
      if (eDefenderAttack == eWhite) return eOutcome_SuperEffective;  // Terra > Luz
      if (eDefenderAttack == eRed) return eOutcome_NotEffective;      // Terra < Fogo
      break;

    case eGreen: // AR
      if (eDefenderAttack == eYellow) return eOutcome_SuperEffective; // Ar > Terra
      if (eDefenderAttack == eRed) return eOutcome_NotEffective;      // Ar < Fogo
      break;
    
    case eWhite: // LUZ
      if (eDefenderAttack == eBlack) return eOutcome_SuperEffective; // Luz > Sombra
      if (eDefenderAttack == eYellow) return eOutcome_NotEffective;  // Luz < Terra
      break;

    case eBlack: // SOMBRA
      if (eDefenderAttack == eWhite) return eOutcome_SuperEffective; // Sombra > Luz
      if (eDefenderAttack == eRed) return eOutcome_NotEffective;     // Sombra < Fogo
      break;

    default:
      return eOutcome_Neutral;
  }
  return eOutcome_Neutral; // Retorno padrão se nenhuma regra se aplicar
}