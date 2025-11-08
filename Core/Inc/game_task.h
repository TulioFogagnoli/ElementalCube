#ifndef GAME_TASK_H
#define GAME_TASK_H

#include "cmsis_os.h"

/**
 * @brief Tarefa principal da lógica de jogo (Máquina de Estados).
 * Espera por mensagens na 'g_inputQueueHandle'.
 * Processa a lógica do jogo.
 * Envia o estado de renderização para 'g_displayQueueHandle'.
 */
void StartGameTask(void const * argument);

#endif // GAME_TASK_H