#include "game_task.h"
#include "game_types.h"
#include "game_fsm.h"
#include "TCS3472.h"
#include "main.h"
#include <string.h>     
#include <stdlib.h>     

extern osMessageQId  g_inputQueueHandle;
extern osPoolId      g_inputPoolHandle;
extern osMessageQId  g_displayQueueHandle;
extern osPoolId      g_displayPoolHandle;
extern volatile TCS3472_Data g_vColorData[4]; // Para ler os sensores
extern osMutexId     g_sensorDataMutexHandle;


static EGameStates  s_eCurrentState = eInitGame;
static int          s_nSelectedOption = 0;
static EDificult    s_eSelectedDifficulty;
static EWizard      s_eUserPlayer;
static EWizard      s_eCpuPlayer;
static uint8_t      s_u8ContAttack = 0; 

const char* difficultyOptions[MENU_OPTIONS_DIFFICULTY] = {"Facil", "Medio", "Dificil"};
const char* personaOptions[MENU_OPTIONS_PERSONA] = {"Mago de Fogo", "Mago de Agua", "Mago de Terra", "Mago de Ar", "Mago da Luz"};



/**
 * @brief Função helper para preencher e enviar o estado atual
 * para a fila da DisplayTask.
 */
static void vSendDisplayState(uint8_t sdCardState) // Recebe o status do SD
{
  DisplayState_t* pDisplayState = osPoolAlloc(g_displayPoolHandle);

  if (pDisplayState != NULL)
  {
      pDisplayState->eStateToRender = s_eCurrentState;
      pDisplayState->nSelectedOption = s_nSelectedOption;
      pDisplayState->u8SdCardMounted = sdCardState; // Passa o status
      
      memcpy(&pDisplayState->stUserPlayer, &s_eUserPlayer, sizeof(EWizard));
      memcpy(&pDisplayState->stCpuPlayer, &s_eCpuPlayer, sizeof(EWizard));
      
      osMessagePut(g_displayQueueHandle, (uint32_t)pDisplayState, 0);
  }
}

/**
 * @brief Processa a máquina de estados com base na tecla recebida.
 * Esta é a sua antiga lógica da 'StartGameTask'.
 */
static void vProcessGameFSM(char cKeyPressed, uint8_t sdCardState)
{
    // Flag para sabermos se precisamos atualizar o display
    uint8_t bStateChanged = 0;

    switch(s_eCurrentState)
    {
      case eInitGame:
      {
        s_eUserPlayer.u8HeartPoints = 100;
        s_eCpuPlayer.u8HeartPoints = 100;
        if(cKeyPressed == CONFIRM_KEY)
        {
          s_eCurrentState = eDificultSelect;
          s_nSelectedOption = 0;
          bStateChanged = 1;
        }
        break;
      }
      case eDificultSelect:
      {
        switch(cKeyPressed)
        {
          case UP_KEY:
            s_nSelectedOption = (s_nSelectedOption < MENU_OPTIONS_DIFFICULTY - 1) ? s_nSelectedOption + 1 : 0;
            bStateChanged = 1;
            break;
          case DOWN_KEY:
            s_nSelectedOption = (s_nSelectedOption > 0) ? s_nSelectedOption - 1 : MENU_OPTIONS_DIFFICULTY - 1;
            bStateChanged = 1;
            break;
          case BACK_KEY:
            s_eCurrentState = eInitGame;
            bStateChanged = 1;
            break;
          case CONFIRM_KEY:
            s_eSelectedDifficulty = (EDificult)s_nSelectedOption;
            s_eCurrentState = ePersonaSelect;
            s_nSelectedOption = 0; // Reseta para o próximo menu
            bStateChanged = 1;
            break;
          default: break;
        }
        break;
      }
      case ePersonaSelect:
      {
        switch(cKeyPressed)
        {
          case UP_KEY:
            s_nSelectedOption = (s_nSelectedOption < MENU_OPTIONS_PERSONA - 1) ? s_nSelectedOption + 1 : 0;
            bStateChanged = 1;
            break;
          case DOWN_KEY:
            s_nSelectedOption = (s_nSelectedOption > 0) ? s_nSelectedOption - 1 : MENU_OPTIONS_PERSONA - 1;
            bStateChanged = 1;
            break;
          case BACK_KEY:
            s_eCurrentState = eDificultSelect;
            bStateChanged = 1;
            break;
          case CONFIRM_KEY:
            s_eUserPlayer.ePersonaElemental = (EElemental)s_nSelectedOption;
            s_eCurrentState = eBattleInit;
            bStateChanged = 1;
            s_u8ContAttack = 0;
            s_nSelectedOption = 0;

            memset((void*)s_eUserPlayer.eAttackSequential, 0, sizeof(s_eUserPlayer.eAttackSequential));
            memset((void*)s_eCpuPlayer.eAttackSequential, 0, sizeof(s_eCpuPlayer.eAttackSequential));

            srand(HAL_GetTick()); 
            s_eCpuPlayer.ePersonaElemental = (rand() % 6);
            for(uint8_t u8Idx = 0; u8Idx < ATTACKS_NUMBERS; u8Idx++)
            {
              s_eCpuPlayer.eAttackSequential[u8Idx] = (EColor)(rand() % 6); 
            }
            break;
          default: break;
        }
        break;
      }
      case eBattleInit:
      {
        switch (cKeyPressed)
        {
          case CONFIRM_KEY:
          {
            if (osMutexWait(g_sensorDataMutexHandle, 100) == osOK)
            {
              for (int i = 0; i < ATTACKS_NUMBERS; i++)
              {
                  s_eUserPlayer.eAttackSequential[i] = TCS3472_DetectColor(g_vColorData[i]);
              }
              osMutexRelease(g_sensorDataMutexHandle);
            }
            vInitBattle(&s_eUserPlayer, &s_eCpuPlayer);
            s_eCurrentState = ePlayerTurn;
            bStateChanged = 1;
            break;
          }
          case BACK_KEY:
          {
            s_eCurrentState = ePersonaSelect;
            bStateChanged = 1;
            break;
          }
          default: break;
        }
        break;
      }
      case ePlayerTurn:
      {
        if(cKeyPressed == CONFIRM_KEY)
        {            
          if (s_eUserPlayer.u8HeartPoints == 0 || s_eCpuPlayer.u8HeartPoints == 0)
          {
            s_eCurrentState = eEndGame; 
          }
          else
          {
            s_eCurrentState = eBattleInit;
            s_u8ContAttack = 0;
            s_nSelectedOption = 0;
            memset((void*)s_eUserPlayer.eAttackSequential, 0, sizeof(s_eUserPlayer.eAttackSequential));
            
            for(uint8_t u8Idx = 0; u8Idx < ATTACKS_NUMBERS; u8Idx++)
            {
              s_eCpuPlayer.eAttackSequential[u8Idx] = (EColor)(rand() % 6);
            }
          }
          bStateChanged = 1;
        }
        break;
      }
      case eEndGame:
      {
        if(cKeyPressed == CONFIRM_KEY)
        {
          s_eCurrentState = eInitGame;
          bStateChanged = 1;
        }
        break;
      }
      default:
      {
        s_eCurrentState = eInitGame;
        bStateChanged = 1;
        break;
      }
    }

    if (bStateChanged)
    {
        vSendDisplayState(sdCardState); // Envia o novo estado
    }
}


/**
 * @brief Tarefa da Lógica do Jogo (O Cérebro)
 */
void StartGameTask(void const * argument)
{
osEvent stEvent;
  
  // Lê o status do SD card, passado como argumento pelo main.c
  uint8_t u8SdCardStatus = (uint8_t)((uint32_t)argument);
  
  // REMOVA o osSemaphoreWait
  
  s_eCurrentState = eInitGame;
  vSendDisplayState(u8SdCardStatus); // Envia o estado inicial (eInitGame)

  for(;;)
  {
    stEvent = osMessageGet(g_inputQueueHandle, osWaitForever);
    
    if(stEvent.status == osEventMessage)
    {
      InputEvent_t* pReceivedEvent = (InputEvent_t*)stEvent.value.p;
      
      if(pReceivedEvent->eType == INPUT_EVENT_KEYPAD)
      {
        vProcessGameFSM(pReceivedEvent->cValue, u8SdCardStatus);
      }
      
      osPoolFree(g_inputPoolHandle, pReceivedEvent);
    }
  }}
