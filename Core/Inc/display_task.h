#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include "cmsis_os.h"

/**
 * @brief Tarefa de Renderização do Display (O Pintor).
 * Inicializa os periféricos de aplicação (Display, MUX, Sensores, SD).
 * Espera por mensagens 'DisplayState_t' na 'g_displayQueueHandle'.
 * Desenha o estado recebido na tela.
 * Lida com atualizações em tempo real (ex: cores do sensor).
 */
void StartDisplayTask(void const * argument);

#endif // DISPLAY_TASK_H