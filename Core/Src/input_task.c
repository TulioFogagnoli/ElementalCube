#include "input_task.h"
#include "game_types.h"
#include "main.h"     
#include "keypad.h"     
#include "TCS3472.h"    
#include <string.h>    

extern osMessageQId g_inputQueueHandle;
extern osMutexId    g_sensorDataMutexHandle;
extern volatile TCS3472_Data g_vColorData[4];
extern I2C_HandleTypeDef hi2c2;
extern osPoolId     g_inputPoolHandle;
/**
 * @brief Tarefa que lê o teclado.
 */
void StartKeypadTask(void const * argument)
{
  char cCurrent;
  
  for(;;)
  {
    cCurrent = KEYPAD_Scan(); // Escaneia o teclado
    if(cCurrent != '\0')
    {
      InputEvent_t* pEvent = osPoolAlloc(g_inputPoolHandle);

      if (pEvent != NULL) 
      {
        pEvent->eType = INPUT_EVENT_KEYPAD;
        pEvent->cValue = cCurrent;
        
        osMessagePut(g_inputQueueHandle, (uint32_t)pEvent, 100);
      }    }
    osDelay(50); // Debounce / Polling rate
  }
}

/**
 * @brief Tarefa que lê os 4 sensores de cor continuamente.
 */
void StartSensorTask(void const * argument)
{
  TCS3472_Data vLocalColorData[4]; // Buffer local
  
  for(;;)
  {
    for (int i = 0; i < 4; i++) {
        TCS3472_ReadData(&hi2c2, i, &vLocalColorData[i]);
    }
    
    if (osMutexWait(g_sensorDataMutexHandle, 100) == osOK)
    {
        memcpy((void*)g_vColorData, vLocalColorData, sizeof(g_vColorData));
        
        osMutexRelease(g_sensorDataMutexHandle);
    }
    
    osDelay(50); // Taxa de atualização dos sensores
  }
}