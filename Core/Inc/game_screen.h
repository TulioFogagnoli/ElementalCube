#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include <stdint.h>
#include "game_types.h"

#define FRAME_PLAYER_W      85   // Exemplo: um pouco maior que 70
#define FRAME_PLAYER_H      85
#define FRAME_CPU_W         35   // Exemplo: um pouco maior que 35
#define FRAME_CPU_H         35

// --- OFFSET (AJUSTE FINO) ---
// Quanto a moldura deve "voltar" em X e Y em relação ao ícone para ficar centralizada?
// Exemplo: Se a moldura tem 84 e o ícone 70 -> (84 - 70) / 2 = 7 pixels de borda
#define FRAME_PLAYER_OFF_X  7
#define FRAME_PLAYER_OFF_Y  7

#define FRAME_CPU_OFF_X     3  // (42 - 35) / 2 = ~3.5
#define FRAME_CPU_OFF_Y     3

#define PLAYER_ICON_W       70  
#define PLAYER_ICON_H       70
#define CPU_ICON_W          35  
#define CPU_ICON_H          35

// Menu Centro / Battle Player (mgF2.bin -> 170x230)
#define WIZ_PLAYER_W        171 
#define WIZ_PLAYER_H        231

// Menu Lateral (mgF1.bin -> 123x167)
#define WIZ_MENU_SM_W       124
#define WIZ_MENU_SM_H       168

// Battle CPU (mgF3.bin -> 78x106)
#define WIZ_CPU_W           78
#define WIZ_CPU_H           106

#define NUM_ELEMENTS        6 

#define ICON_SIZE_P_BYTES   (PLAYER_ICON_W * PLAYER_ICON_H * 3)
// --- Estruturas de Dados ---
#define SLOT_X_START    75  // Espaço para centralizar 4 slots
#define SLOT_SPACING    86
#define SLOT_CPU_Y      110  // Ícones pequenos em cima
#define SLOT_PLAYER_Y   235 // Ícones grandes em bai

#define Y_POS_BIG       50
#define X_CENTER        (240 - (WIZ_PLAYER_W / 2)) 

// Laterais (123x167) 
// NÃO USE VALORES NEGATIVOS com uint16_t!
#define Y_POS_SMALL     80
#define X_LEFT          5    // Colado na esquerda (antes era -30)
#define X_RIGHT         352  // Colado na direita (480 - 123 - 5)

// --- Tela de Batalha ---
// Player (170x230)
#define BTL_PLAYER_X    10    // Colado na esquerda (antes era -20)
#define BTL_PLAYER_Y    80   
// CPU (78x106)
#define BTL_CPU_X       200  // Canto direito
#define BTL_CPU_Y       60

// Estrutura atualizada com 4 campos
typedef struct {
    const char* name;         // Nome (Ex: "Fogo")
    const char* path_menu_sm; // Menu Pequeno (Ex: mgF1.bin)
    const char* path_menu_bg; // Menu Grande / Player Battle (Ex: mgF2.bin)
    const char* path_cpu;     // CPU Battle (Ex: mgF3.bin)
} PersonaOptionData_t;

typedef struct {
    const char* norm_path;
    const char* sel_path;
    uint16_t x_norm;
    uint16_t y_norm;
    uint16_t w_norm;
    uint16_t h_norm;
    uint16_t w_sel;
    uint16_t h_sel;
} DifficultyOptionData_t;

extern uint8_t PlayerIconCache[NUM_ELEMENTS][ICON_SIZE_P_BYTES];

// --- Protótipos de Funções ---
void ClearScreen();
void DrawMenu(const char* title, const char** options, int numOptions, int currentSelection);
void DrawDifficultyMenu(int currentSelection);
void DrawSingleDifficultyOption(int index, int isSelected);
void DrawPersonaCarousel(int selectedIndex);

uint8_t LoadAllIconsToCache(void);
void DrawBattleLayout(EDificult difficulty, const EWizard* user, const EWizard* cpu);
void UpdatePlayerAttacks(const EWizard* user);

#endif // GAME_SCREEN_H