#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>

typedef enum {
    eElementalFire, 
    eElementalWater, 
    eElementalEarth, 
    eElementalAir, 
    eElementalLight, 
    eElementalShadow
} EElemental;

typedef enum {
    eRed,
    eBlue,
    eGreen,
    eYellow,
    eWhite,
    eBlack
} EColor;

typedef enum {
    eDificultEasy,
    eDificultMedium,
    eDificultHard
} EDificult;

typedef struct {
    uint8_t u8HeartPoints;
    EElemental ePersonaElemental;
    EColor eAttackSequential[4];
} EWizard;

#endif //GAME_TYPES_H