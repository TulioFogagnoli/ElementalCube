#include "game_screen.h"
#include "game_fsm.h"
#include "ILI9488.h"
#include "fonts.h"
#include <stdio.h>
#include "main.h"
#include "fatfs.h"

// =========================================================
// I. DADOS E CACHES
// =========================================================

uint8_t PlayerIconCache[NUM_ELEMENTS][ICON_SIZE_P_BYTES];

// Caminhos Ícones 70x70 (Player) - Baseado no log 
const char* ICON_PATHS_BIG[] = {
    "0:/Fo2.bin", "0:/Ag2.bin", "0:/T2.bin", 
    "0:/Ar2.bin", "0:/Lu2.bin", "0:/So2.bin"
};

// Caminhos Ícones 35x35 (CPU) - Baseado no log
const char* ICON_PATHS_SMALL[] = {
    "0:/F1.bin", "0:/Ag1.bin", "0:/T1.bin", 
    "0:/Ar1.bin", "0:/L1.bin", "0:/S1.bin"
};

// Tabela Mestra de Personagens (Baseado em mgF1, mgF2, mgF3...)
const PersonaOptionData_t PERSONA_DATA[] = {
    // Nome,   Menu_Sm (123x167), Menu_Bg/Play (170x230), CPU (78x106)
    {"Fogo",  "0:/mgF1.bin",  "0:/mgF2.bin",  "0:/mgF3.bin"},
    {"Agua",  "0:/mgAg1.bin", "0:/mgAg2.bin", "0:/mgAg3.bin"},
    {"Terra", "0:/mgT1.bin",  "0:/mgT2.bin",  "0:/mgT3.bin"},
    {"Ar",    "0:/mgAr1.bin", "0:/mgAr2.bin", "0:/mgAr3.bin"},
    {"Luz",   "0:/mgL1.bin",  "0:/mgL2.bin",  "0:/mgL3.bin"},
    {"Sombra","0:/mgS1.bin",  "0:/mgS2.bin",  "0:/mgS3.bin"}
};

// Tabela de Dificuldade (Baseado em facil.bin, f2.bin...)
// Dimensões Log: facil (262x87), f2 (290x100)
const DifficultyOptionData_t DIFFICULTY_DATA[] = {
    {"0:/facil.bin",   "0:/f2.bin", 110, 20,  262, 87, 290, 100},
    {"0:/Medio.bin",   "0:/m2.bin", 110, 120, 262, 87, 290, 100}, // Atenção: Medio com M maiúsculo no log
    {"0:/dificil.bin", "0:/d2.bin", 110, 220, 262, 87, 290, 100}
};

// =========================================================
// II. FUNÇÕES AUXILIARES
// =========================================================

void ClearScreen() {
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bg.bin"); 
}

void DrawCachedPlayerIcon(uint16_t x, uint16_t y, EColor element) {
    ILI9488_DrawCachedSprite_Transparent(
        x, y, PLAYER_ICON_W, PLAYER_ICON_H, PlayerIconCache[element]);
}

// =========================================================
// III. MENU DIFICULDADE
// =========================================================

void DrawDifficultyMenu(int currentSelection) {
    char buffer[30];
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgd.bin"); 
    
    for (int i = 0; i < 3; i++) {
        const DifficultyOptionData_t* option = &DIFFICULTY_DATA[i];
        
        uint16_t w_draw, h_draw, x_draw, y_draw;
        const char* path_draw;

        if (i == currentSelection) {
            path_draw = option->sel_path;
            w_draw = option->w_sel;
            h_draw = option->h_sel;
            // Centraliza
            x_draw = option->x_norm - ((option->w_sel - option->w_norm) / 2);
            y_draw = option->y_norm - ((option->h_sel - option->h_norm) / 2);
        } else {
            path_draw = option->norm_path;
            w_draw = option->w_norm;
            h_draw = option->h_norm;
            x_draw = option->x_norm;
            y_draw = option->y_norm;
        }

        if (!ILI9488_DrawImage_Transparent(x_draw, y_draw, w_draw, h_draw, path_draw)) {
             sprintf(buffer, "Erro: %s", path_draw);
             ILI9488_WriteString(option->x_norm, option->y_norm, buffer, Font_7x10, ILI9488_RED, ILI9488_BLACK);
        }
    }
}

void DrawSingleDifficultyOption(int index, int isSelected) {
    const DifficultyOptionData_t* option = &DIFFICULTY_DATA[index];

    if (isSelected) {
        uint16_t x = option->x_norm - ((option->w_sel - option->w_norm) / 2);
        uint16_t y = option->y_norm - ((option->h_sel - option->h_norm) / 2);
        ILI9488_DrawImage_Transparent(x, y, option->w_sel, option->h_sel, option->sel_path);
    } else {
        // Restaura fundo
        uint16_t x_sel = option->x_norm - ((option->w_sel - option->w_norm) / 2);
        uint16_t y_sel = option->y_norm - ((option->h_sel - option->h_norm) / 2);
        ILI9488_RestoreRect(x_sel, y_sel, option->w_sel, option->h_sel, "0:/bgd.bin");
        
        // Desenha pequeno
        ILI9488_DrawImage_Transparent(option->x_norm, option->y_norm, option->w_norm, option->h_norm, option->norm_path);
    }
}

// =========================================================
// IV. MENU CARROSSEL (PERSONAGEM)
// =========================================================

void DrawPersonaCarousel(int selectedIndex) {
    int idx_left  = (selectedIndex - 1);
    if (idx_left < 0) idx_left = 5; // Ajuste para 6 elementos (0 a 5)

    int idx_right = (selectedIndex + 1);
    if (idx_right >= 6) idx_right = 0;

    // Limpa áreas (Restaurando bg.bin)
    ILI9488_RestoreRect(X_LEFT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, "0:/bg.bin");
    ILI9488_RestoreRect(X_RIGHT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, "0:/bg.bin");
    ILI9488_RestoreRect(X_CENTER, Y_POS_BIG, WIZ_PLAYER_W, WIZ_PLAYER_H, "0:/bg.bin");

    // Desenha Laterais (Pequenos -> path_menu_sm)
    // Agora com X_LEFT e X_RIGHT positivos, eles vão aparecer!
    ILI9488_DrawImage_Transparent(X_LEFT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, PERSONA_DATA[idx_left].path_menu_sm);
    ILI9488_DrawImage_Transparent(X_RIGHT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, PERSONA_DATA[idx_right].path_menu_sm);

    // Desenha Centro (Grande -> path_menu_bg)
    ILI9488_DrawImage_Transparent(X_CENTER, Y_POS_BIG, WIZ_PLAYER_W, WIZ_PLAYER_H, PERSONA_DATA[selectedIndex].path_menu_bg);
}

// =========================================================
// V. TELA DE BATALHA
// =========================================================

uint8_t LoadAllIconsToCache(void) {
    FIL file;
    UINT br;
    
    // Carrega Ícones Player (70x70) para RAM
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        if (f_open(&file, ICON_PATHS_BIG[i], FA_READ) != FR_OK) continue; 
        
        if (f_read(&file, PlayerIconCache[i], ICON_SIZE_P_BYTES, &br) != FR_OK || br != ICON_SIZE_P_BYTES) {
            f_close(&file);
            return 0; 
        }
        f_close(&file);
    }
    return 1; 
}

void DrawBattleLayout(EDificult difficulty, const EWizard* user, const EWizard* cpu) {
    
    // 1. Background da Batalha
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgc.bin"); 

    // 2. Mago Player (Grande 170x230)
    if (user->ePersonaElemental < NUM_ELEMENTS) {
        const char* pPath = PERSONA_DATA[user->ePersonaElemental].path_menu_bg;
        ILI9488_DrawImage_Transparent(BTL_PLAYER_X, BTL_PLAYER_Y, WIZ_PLAYER_W, WIZ_PLAYER_H, pPath);
    }

    // 3. Mago CPU (Pequeno 78x106)
    if (cpu->ePersonaElemental < NUM_ELEMENTS) {
        const char* cPath = PERSONA_DATA[cpu->ePersonaElemental].path_cpu;
        ILI9488_DrawImage_Transparent(BTL_CPU_X, BTL_CPU_Y, WIZ_CPU_W, WIZ_CPU_H, cPath);
    }

    // 4. Ícones CPU (Estáticos 35x35)
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        uint16_t x = SLOT_X_START + (i * SLOT_SPACING);
        bool draw = false;

        if (difficulty == eDificultEasy) draw = true;
        else if (difficulty == eDificultMedium) draw = (i < 2);

        if (draw && cpu->eAttackSequential[i] < NUM_ELEMENTS) {
            const char* path_cpu_icon = ICON_PATHS_SMALL[cpu->eAttackSequential[i]];
            // Ajuste centralizado (slot 85px, icone 35px -> offset 25)
            ILI9488_DrawImage_Transparent(x + 25, SLOT_CPU_Y, CPU_ICON_W, CPU_ICON_H, path_cpu_icon);
        }
    }
}

void UpdatePlayerAttacks(const EWizard* user) {
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        uint16_t x = SLOT_X_START + (i * SLOT_SPACING);
        EColor elem = user->eAttackSequential[i];

        if (elem < NUM_ELEMENTS) {
            // Desenha do Cache RAM (70x70)
            // Ajuste centralizado (slot 85px, icone 70px -> offset 7)
            DrawCachedPlayerIcon(x + 7, SLOT_PLAYER_Y, elem);
        } else {
            // Limpa slot restaurando bgc.bin
            ILI9488_RestoreRect(x + 7, SLOT_PLAYER_Y, PLAYER_ICON_W, PLAYER_ICON_H, "0:/bgc.bin");
        }
    }
}