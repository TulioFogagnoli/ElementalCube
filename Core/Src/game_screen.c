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
    "0:/Fo2.bin", "0:/Ag2.bin", "0:/Te2.bin", 
    "0:/Ar2.bin", "0:/Lu2.bin", "0:/So2.bin"
};

// Caminhos Ícones 35x35 (CPU) - Baseado no log
const char* ICON_PATHS_SMALL[] = {
    "0:/Fo1.bin", "0:/Ag1.bin", "0:/Te1.bin", 
    "0:/Ar1.bin", "0:/Lu1.bin", "0:/So1.bin"
};

// Tabela Mestra de Personagens (Baseado em mgF1, mgF2, mgF3...)
const PersonaOptionData_t PERSONA_DATA[] = {
    // Nome,   Menu_Sm (123x167), Menu_Bg/Play (170x230), CPU (78x106)
    {"Fogo",  "0:/mgFo2.bin",  "0:/mgFo3.bin",  "0:/mgFo1.bin"},
    {"Agua",  "0:/mgAg2.bin", "0:/mgAg3.bin", "0:/mgAg1.bin"},
    {"Terra", "0:/mgTe2.bin",  "0:/mgTe3.bin",  "0:/mgTe1.bin"},
    {"Ar",    "0:/mgAr2.bin", "0:/mgAr3.bin", "0:/mgAr1.bin"},
    {"Luz",   "0:/mgLu2.bin",  "0:/mgLu3.bin",  "0:/mgLu1.bin"},
    {"Sombra","0:/mgSo2.bin",  "0:/mgSo3.bin",  "0:/mgSo1.bin"}
};

// Tabela de Dificuldade (Baseado em facil.bin, f2.bin...)
// Dimensões Log: facil (262x87), f2 (290x100)
const DifficultyOptionData_t DIFFICULTY_DATA[] = {
    {"0:/f2.bin",   "0:/f2.bin", 125, 20,  233, 94, 233, 94},
    {"0:/m2.bin",   "0:/m2.bin", 125, 115, 233, 94, 233, 94}, // Atenção: Medio com M maiúsculo no log
    {"0:/d2.bin", "0:/d2.bin", 125, 200, 233, 94, 233, 94}
};

const uint16_t PLAYER_SLOTS_POS[4][2] = {
    {73,  236}, // Slot 0
    {160, 236}, // Slot 1
    {250, 236}, // Slot 2
    {340, 236}  // Slot 3
};

// Slots da CPU (Ícones Pequenos 35x35)
const uint16_t CPU_SLOTS_POS[4][2] = {
    {165, 150}, // Slot 0 (Geralmente centralizado em relação ao de baixo)
    {203, 150}, // Slot 1
    {238, 150}, // Slot 2
    {275, 150}  // Slot 3
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
    // 1. Desenha o fundo (que AGORA JÁ CONTÉM os 3 botões pequenos desenhados nele)
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgd.bin"); 
    
    // 2. Desenha APENAS a opção selecionada na versão "Grande"
    // Não precisamos desenhar os não-selecionados, pois já estão no bgd.bin
    const DifficultyOptionData_t* option = &DIFFICULTY_DATA[currentSelection];
    
    uint16_t x_sel = option->x_norm - ((option->w_sel - option->w_norm) / 2);
    uint16_t y_sel = option->y_norm - ((option->h_sel - option->h_norm) / 2);

    ILI9488_DrawImage_Transparent(x_sel, y_sel, option->w_sel, option->h_sel, option->sel_path);
}

void DrawSingleDifficultyOption(int index, int isSelected) {
    const DifficultyOptionData_t* option = &DIFFICULTY_DATA[index];

    // Cálculos da posição do ícone GRANDE (centralizado sobre o pequeno)
    uint16_t x_sel = option->x_norm - ((option->w_sel - option->w_norm) / 2);
    uint16_t y_sel = option->y_norm - ((option->h_sel - option->h_norm) / 2);

    if (isSelected) {
        // SELECIONANDO: Desenha o ícone Grande por cima do fundo
        ILI9488_DrawImage_Transparent(x_sel, y_sel, option->w_sel, option->h_sel, option->sel_path);
    } else {
        // DESELECIONANDO: Apenas restaura o background na área do ícone Grande.
        // Como o 'bgd.bin' já tem o ícone pequeno desenhado, ao restaurar o fundo,
        // o ícone grande some e o pequeno "aparece".
        ILI9488_RestoreRect(x_sel, y_sel, option->w_sel, option->h_sel, "0:/bgd.bin");
    }
}

// =========================================================
// IV. MENU CARROSSEL (PERSONAGEM)
// =========================================================

void DrawPersonaCarousel(int selectedIndex) {

    int idx_left  = (selectedIndex - 1);
    if (idx_left < 0) idx_left = 5; 

    int idx_right = (selectedIndex + 1);
    if (idx_right >= 6) idx_right = 0;

    // Limpa áreas usando a Borracha (Restaurando o fundo que já está lá)
    ILI9488_RestoreRect(X_LEFT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, "0:/bgp.bin");
    ILI9488_RestoreRect(X_RIGHT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, "0:/bgp.bin");
    ILI9488_RestoreRect(X_CENTER, Y_POS_BIG, WIZ_PLAYER_W, WIZ_PLAYER_H, "0:/bgp.bin");

    // Desenha os Sprites
    ILI9488_DrawImage_Transparent(X_LEFT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, PERSONA_DATA[idx_left].path_menu_sm);
    ILI9488_DrawImage_Transparent(X_RIGHT, Y_POS_SMALL, WIZ_MENU_SM_W, WIZ_MENU_SM_H, PERSONA_DATA[idx_right].path_menu_sm);
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
    
    // 1. Background (Camada 0)
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgc.bin"); 

    // 2. Sprites dos Magos (Camada 1 - Desenhados por cima do fundo)
    if (user->ePersonaElemental < NUM_ELEMENTS) {
        const char* pPath = PERSONA_DATA[user->ePersonaElemental].path_menu_bg;
        ILI9488_DrawImage_Transparent(BTL_PLAYER_X, BTL_PLAYER_Y, WIZ_PLAYER_W, WIZ_PLAYER_H, pPath);
    }

    if (cpu->ePersonaElemental < NUM_ELEMENTS) {
        const char* cPath = PERSONA_DATA[cpu->ePersonaElemental].path_cpu;
        ILI9488_DrawImage_Transparent(BTL_CPU_X, BTL_CPU_Y, WIZ_CPU_W, WIZ_CPU_H, cPath);
    }

    // 3. TRUQUE DE CAMADAS: Restaurar as Molduras (Camada 2)
    // Aqui usamos as dimensões DA MOLDURA (maiores), não do ícone.
    // Isso recorta o mago e traz a moldura do fundo para frente.
    
    // Restaura Molduras do Player
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        // Calcula a posição da moldura baseada na posição do ícone - offset
        uint16_t frame_x = PLAYER_SLOTS_POS[i][0] - FRAME_PLAYER_OFF_X;
        uint16_t frame_y = PLAYER_SLOTS_POS[i][1] - FRAME_PLAYER_OFF_Y;

        ILI9488_RestoreRect(
            frame_x, frame_y, 
            FRAME_PLAYER_W, FRAME_PLAYER_H, 
            "0:/bgc.bin"
        );
    }

    // Restaura Molduras da CPU
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        uint16_t frame_x = CPU_SLOTS_POS[i][0] - FRAME_CPU_OFF_X;
        uint16_t frame_y = CPU_SLOTS_POS[i][1] - FRAME_CPU_OFF_Y;

        ILI9488_RestoreRect(
            frame_x, frame_y, 
            FRAME_CPU_W, FRAME_CPU_H, 
            "0:/bgc.bin"
        );
    }

    // 4. Ícones de Ataque da CPU (Camada 3 - Ficam DENTRO da moldura restaurada)
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        bool draw = false;
        if (difficulty == eDificultEasy) draw = true;
        else if (difficulty == eDificultMedium) draw = (i < 2);

        if (draw && cpu->eAttackSequential[i] < NUM_ELEMENTS) {
            const char* path_cpu_icon = ICON_PATHS_SMALL[cpu->eAttackSequential[i]];
            
            // Desenha o ícone na posição original (centralizado na moldura restaurada)
            uint16_t x = CPU_SLOTS_POS[i][0];
            uint16_t y = CPU_SLOTS_POS[i][1];
            
            ILI9488_DrawImage_Transparent(x, y, CPU_ICON_W, CPU_ICON_H, path_cpu_icon);
        }
    }
}

void UpdatePlayerAttacks(const EWizard* user) {
    // Variável estática para lembrar o estado anterior dos slots
    // Inicializada com um valor impossível (0xFF) para forçar o desenho na primeira vez
    static EColor last_attacks[ATTACKS_NUMBERS] = {0xFF, 0xFF, 0xFF, 0xFF};

    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        EColor current_element = user->eAttackSequential[i];

        // 🛑 OTIMIZAÇÃO CRÍTICA: Só desenha se houver mudança!
        // Isso evita ler o SD card repetidamente se o sensor não mudou.
        if (current_element != last_attacks[i]) {
            
            // 1. Recupera a posição do ÍCONE (central)
            uint16_t icon_x = PLAYER_SLOTS_POS[i][0];
            uint16_t icon_y = PLAYER_SLOTS_POS[i][1];

            // 2. Calcula a posição da MOLDURA (b1.bin)
            // Recuamos 7 pixels em X e Y para cobrir toda a área antiga
            uint16_t frame_x = icon_x - FRAME_PLAYER_OFF_X;
            uint16_t frame_y = icon_y - FRAME_PLAYER_OFF_Y;

            // 3. PASSO DA BORRACHA: Desenha a moldura vazia (b1.bin)
            // Isso apaga instantaneamente qualquer ícone que estava lá antes (Ghosting resolvido!)
            ILI9488_DrawImage_Transparent(frame_x, frame_y, FRAME_PLAYER_W, FRAME_PLAYER_H, "0:/b1.bin");

            // 4. PASSO DO ÍCONE: Desenha o novo ataque (se existir)
            if (current_element < NUM_ELEMENTS) {
                // Desenha o ícone 70x70 do Cache RAM por cima da moldura limpa
                DrawCachedPlayerIcon(icon_x, icon_y, current_element);
            }

            // 5. Atualiza o histórico
            last_attacks[i] = current_element;
        }
    }
}