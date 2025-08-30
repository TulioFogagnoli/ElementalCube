#ifndef KEYPAD_H
#define KEYPAD_H

#include "stm32f4xx_hal.h"

// Função que retorna o caractere pressionado ou '\0' se nada for pressionado
char KEYPAD_Scan(void);

#endif // KEYPAD_H