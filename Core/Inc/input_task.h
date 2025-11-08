#ifndef INPUT_TASK_H
#define INPUT_TASK_H

#include "cmsis_os.h"

/**
 * @brief Tarefa que lê o teclado (keypad).
 * Em caso de tecla pressionada, envia um 'InputEvent_t'
 * para a fila 'g_inputQueueHandle'.
 */
void StartKeypadTask(void const * argument);

/**
 * @brief Tarefa que lê os 4 sensores de cor continuamente.
 * Atualiza a variável global 'g_vColorData' e protege
 * o acesso com o mutex 'g_sensorDataMutexHandle'.
 */
void StartSensorTask(void const * argument);

#endif // INPUT_TASK_H