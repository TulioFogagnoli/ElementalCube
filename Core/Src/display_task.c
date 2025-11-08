#include "display_task.h"
#include "game_types.h"
#include "main.h"
#include "game_screen.h"
#include "ILI9488.h"
#include "TCS3472.h"
#include "fonts.h"
#include <stdio.h>
#include <string.h>    

extern osMessageQId  g_displayQueueHandle;
extern osPoolId      g_displayPoolHandle;
extern osMutexId     g_sensorDataMutexHandle;
extern volatile TCS3472_Data g_vColorData[4];

extern const char* difficultyOptions[];
extern const char* personaOptions[];
static DisplayState_t s_stCurrentDisplayState;
static uint8_t s_bForceRedraw = 1;

/**
 * 
 * @brief Função especial para a tela eBattleInit.
 * Fica em loop atualizando as cores dos sensores em tempo real,
 * ATÉ que uma nova mensagem chegue na fila (indicando que o
 * estado mudou, ex: o usuário pressionou CONFIRM).
 */
static void vDrawBattleDynamicSensorUpdate(void)
{
  char colorName[10];
  uint16_t colorBox;
  TCS3472_Data vLocalSensorData[4]; 

  if (osMutexWait(g_sensorDataMutexHandle, 50) == osOK)
  {
    memcpy(vLocalSensorData, (void*)g_vColorData, sizeof(vLocalSensorData));
    osMutexRelease(g_sensorDataMutexHandle);
  }
  else
    return;

  for (int i = 0; i < 4; i++) {
    EColor detectedColor = TCS3472_DetectColor(vLocalSensorData[i]);
    colorBox = ILI9488_BLACK;
    strcpy(colorName, "Vazio");
    switch(detectedColor) {
        case eRed:    colorBox = ILI9488_RED;   strcpy(colorName, "Fogo");   break;
        case eBlue:   colorBox = ILI9488_BLUE;  strcpy(colorName, "Agua");   break;
        case eGreen:  colorBox = ILI9488_CYAN;  strcpy(colorName, "Ar");     break;
        case eYellow: colorBox = ILI9488_BROWN; strcpy(colorName, "Terra");  break;
        case eWhite:  colorBox = ILI9488_WHITE; strcpy(colorName, "Luz?");   break;
        default: break;
    }
    ILI9488_FillRectangle(100, 80 + (i * 30), 45, 10, ILI9488_BLACK);
    ILI9488_WriteString(100, 80 + (i * 30), colorName, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
    ILI9488_FillRectangle(150, 75 + (i * 30), 20, 20, colorBox);
  }
}


/**
 * @brief Tarefa de Renderização do Display (O Pintor)
 */
void StartDisplayTask(void const * argument)
{
  osEvent stEvent;
  char    buffer[40]; // Buffer para sprintf

  // Inicializa o estado local com um valor "dummy"
  // para evitar que `eStateToRender` seja lixo no primeiro loop
  memset(&s_stCurrentDisplayState, 0, sizeof(DisplayState_t));
  s_stCurrentDisplayState.eStateToRender = eEndGame; // Estado "Inválido" inicial
  s_bForceRedraw = 1; // Força o primeiro desenho
  
  for(;;)
  {
    // --- PARTE 1: Checar por *novos* estados (da GameTask) ---
    stEvent = osMessageGet(g_displayQueueHandle, 0); // Timeout 0 = não bloquear
    
    if(stEvent.status == osEventMessage)
    {
      DisplayState_t* pNewState = (DisplayState_t*)stEvent.value.p;
      
      // Verifica se o estado REALMENTE mudou
      if (s_stCurrentDisplayState.eStateToRender != pNewState->eStateToRender ||
          s_stCurrentDisplayState.nSelectedOption != pNewState->nSelectedOption ||
          s_stCurrentDisplayState.stUserPlayer.u8HeartPoints != pNewState->stUserPlayer.u8HeartPoints)
      {
         s_bForceRedraw = 1; // Se mudou, força redesenhar
      }

      // Copia os dados novos para nossa variável local
      memcpy(&s_stCurrentDisplayState, pNewState, sizeof(DisplayState_t));
      
      // Libera o bloco de memória de volta para o pool
      osPoolFree(g_displayPoolHandle, pNewState);
    }

    // --- PARTE 2: Lógica de Redesenho (como no main.c original) ---
    if(s_bForceRedraw)
    {
        s_bForceRedraw = 0; // Reseta a flag
        ClearScreen(); 

        switch(s_stCurrentDisplayState.eStateToRender)
        {
            case eInitGame:
              if (s_stCurrentDisplayState.u8SdCardMounted) {
                  if (!ILI9488_DrawImage_BIN(100, 100, 30, 30, "0:/fire30.bin")) {
                     ILI9488_WriteString(0, 10, "Falha ao ler fire30.bin", Font_7x10, ILI9488_RED, ILI9488_BLACK);
                  }
              } else {
                  ILI9488_WriteString(10, 10, "SD Falhou. Lendo flash.", Font_7x10, ILI9488_RED, ILI9488_BLACK);
              }
              ILI9488_WriteString(10, 280, "Pressione * para iniciar", Font_7x10, ILI9488_YELLOW, ILI9488_BLACK);
              break;
            
            case eDificultSelect:
              DrawMenu("Selecione Dificuldade", difficultyOptions, 
                       MENU_OPTIONS_DIFFICULTY, s_stCurrentDisplayState.nSelectedOption);
              break;

            case ePersonaSelect:
              DrawMenu("Selecione Personagem", personaOptions, 
                       MENU_OPTIONS_PERSONA, s_stCurrentDisplayState.nSelectedOption);
              break;

            case eBattleInit:
              ILI9488_WriteString(10, 15, "Prepare seus ataques!", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
              ILI9488_WriteString(10, 35, "Posicione os 4 cubos e pressione *", Font_7x10, ILI9488_YELLOW, ILI9488_BLACK);
              for (int i = 0; i < 4; i++) {
                sprintf(buffer, "Sensor %d:", i + 1);
                ILI9488_WriteString(20, 80 + (i * 30), buffer, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
              }
              break;

            case ePlayerTurn:
              // ... (Sua lógica de desenho do ePlayerTurn) ...
              break;

            case eEndGame:
              // ... (Sua lógica de desenho do eEndGame) ...
              break;

            default:
              // Isso SÓ deve acontecer na primeira fração de segundo
              // antes da game_task enviar o primeiro estado.
              ILI9488_WriteString(10, 10, "Aguardando Jogo...", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
              break;
        }
    }

    // --- PARTE 3: Lógica de atualização em tempo real ---
    if (s_stCurrentDisplayState.eStateToRender == eBattleInit)
    {
        vDrawBattleDynamicSensorUpdate();
    }
    
    osDelay(50); // Delay de refresh da UI
  }
}