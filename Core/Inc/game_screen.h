#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include <stdint.h>
#include "game_types.h"

// ==============================================================================
// 1. DEFINIÇÕES DE AÇÕES
// ==============================================================================
#define ACTION_IDLE   0
#define ACTION_ATK    1
#define ACTION_HURT   2
#define ACTION_WIN    3
#define ACTION_LOSE   4

// ==============================================================================
// 2. DIMENSÕES DOS SPRITES (Baseado no Log do Python)
// ==============================================================================

// --- PLAYER (Mago Grande) ---
// Usado no Layout estático (mg..3.bin)
#define DIM_PLAYER_BATTLE_W     171
#define DIM_PLAYER_BATTLE_H     231

// Usados na Animação
#define DIM_PLAYER_IDLE_W       129  // mg..I.bin
#define DIM_PLAYER_IDLE_H       164
#define DIM_PLAYER_ATK_W        236  // mg..At.bin
#define DIM_PLAYER_ATK_H        148
#define DIM_PLAYER_DMG_W        166  // mg..Dan.bin
#define DIM_PLAYER_DMG_H        163
// Novos defines para Vitória/Derrota (mg..Vir.bin / mg..Der.bin)
#define DIM_PLAYER_RES_W        166
#define DIM_PLAYER_RES_H        167  // ALTURA MÁXIMA! Importante para limpar tela.

// --- CPU (Mago Pequeno) ---
#define DIM_CPU_IDLE_W          78   // mg..I2.bin
#define DIM_CPU_IDLE_H          106
#define DIM_CPU_ATK_W           183  // mg..At2.bin
#define DIM_CPU_ATK_H           106
#define DIM_CPU_DMG_W           110  // mg..Dan2.bin / Vir2 / Der2 (Todos parecem ter ~110x106)
#define DIM_CPU_DMG_H           106

// --- ÍCONES ---
#define DIM_ICON_BIG_SIZE       70
#define DIM_ICON_SMALL_SIZE     35
#define DIM_ICON_CACHE_SIZE     (DIM_ICON_BIG_SIZE * DIM_ICON_BIG_SIZE * 3)

// --- MOLDURAS ---
#define DIM_FRAME_PLAYER_W      85
#define DIM_FRAME_PLAYER_H      85
#define DIM_FRAME_CPU_W         35
#define DIM_FRAME_CPU_H         35
#define DIM_MENU_SM_W           124
#define DIM_MENU_SM_H           168

// ==============================================================================
// 3. POSICIONAMENTO
// ==============================================================================

// --- ÂNCORAS (Pé no Chão) ---
#define POS_ANCHOR_PLAYER_X     5
#define POS_ANCHOR_PLAYER_Y     310 // Com altura 167, desenha em Y=83. Seguro.

#define POS_ANCHOR_CPU_X        246
#define POS_ANCHOR_CPU_Y        193

#define POS_ANCHOR_CPU_ATK_X    175
#define POS_ANCHOR_CPU_ATK_Y    190

// --- HUD: BARRAS DE VIDA ---
#define POS_BAR_PLAYER_X        330
#define POS_BAR_PLAYER_Y        285
#define DIM_BAR_W               131
#define DIM_BAR_H               19

#define POS_BAR_CPU_X           17
#define POS_BAR_CPU_Y           16

// --- DUELO ---
#define POS_CLASH_PLAYER_X      373
#define POS_CLASH_PLAYER_Y      180

#define POS_CLASH_CPU_X         373
#define POS_CLASH_CPU_Y         85

// --- OFFSETS E OUTROS ---
#define OFFSET_FRAME_PLAYER_X   7
#define OFFSET_FRAME_PLAYER_Y   7
#define OFFSET_FRAME_CPU_X      3
#define OFFSET_FRAME_CPU_Y      3
#define POS_SLOT_PLAYER_Y_BASE  236
#define POS_SLOT_CPU_Y_BASE     150
#define POS_MENU_CENTER_X       (240 - (171 / 2)) // Usando 171 direto ou DIM_PLAYER_BATTLE_W
#define POS_MENU_BIG_Y          50
#define POS_MENU_SMALL_Y        80
#define POS_MENU_LEFT_X         5
#define POS_MENU_RIGHT_X        352

// ==============================================================================
// 4. ESTRUTURAS E PROTÓTIPOS
// ==============================================================================
#define NUM_ELEMENTS 6 

typedef struct {
    const char* name;
    const char* path_menu_sm;
    const char* path_menu_bg;
    const char* path_cpu;
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

extern uint8_t PlayerIconCache[NUM_ELEMENTS][DIM_ICON_CACHE_SIZE];

void ClearScreen();
void DrawMenu(const char* title, const char** options, int numOptions, int currentSelection);
void DrawDifficultyMenu(int currentSelection);
void DrawSingleDifficultyOption(int index, int isSelected);
void DrawPersonaCarousel(int selectedIndex);
uint8_t LoadAllIconsToCache(void);

void DrawBattleLayout(EDificult difficulty, const EWizard* user, const EWizard* cpu);
void UpdatePlayerAttacks(const EWizard* user);

void DrawBattleResolutionBg();
void UpdateHealthBars(uint8_t currentHpPlayer, uint8_t currentHpCpu);
void DrawClashIcons(EColor playerColor, EColor cpuColor);
void DrawWizardAction(const EWizard* wiz, uint8_t isPlayer, uint8_t action);
void EraseClashIcons(void);

#endif // GAME_SCREEN_H