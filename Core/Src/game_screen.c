#include "game_screen.h"
#include "game_fsm.h"
#include "ILI9488.h"
#include "fonts.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "fatfs.h"

// =========================================================
// I. DADOS E CACHES
// =========================================================

uint8_t PlayerIconCache[NUM_ELEMENTS][DIM_ICON_CACHE_SIZE];

const char* ICON_PATHS_BIG[] = {
    "0:/Fo2.bin", "0:/Ag2.bin", "0:/Te2.bin", 
    "0:/Ar2.bin", "0:/Lu2.bin", "0:/So2.bin"
};

const char* ICON_PATHS_SMALL[] = {
    "0:/Fo1.bin", "0:/Ag1.bin", "0:/Te1.bin", 
    "0:/Ar1.bin", "0:/Lu1.bin", "0:/So1.bin"
};

const PersonaOptionData_t PERSONA_DATA[] = {
    {"Fogo",  "0:/mgFo2.bin",  "0:/mgFo3.bin",  "0:/mgFo1.bin"},
    {"Agua",  "0:/mgAg2.bin", "0:/mgAg3.bin", "0:/mgAg1.bin"},
    {"Terra", "0:/mgTe2.bin",  "0:/mgTe3.bin",  "0:/mgTe1.bin"},
    {"Ar",    "0:/mgAr2.bin", "0:/mgAr3.bin", "0:/mgAr1.bin"},
    {"Luz",   "0:/mgLu2.bin",  "0:/mgLu3.bin",  "0:/mgLu1.bin"},
    {"Sombra","0:/mgSo2.bin",  "0:/mgSo3.bin",  "0:/mgSo1.bin"}
};

const DifficultyOptionData_t DIFFICULTY_DATA[] = {
    {"0:/f2.bin",   "0:/f2.bin", 125, 20,  233, 94, 233, 94},
    {"0:/m2.bin",   "0:/m2.bin", 125, 115, 233, 94, 233, 94},
    {"0:/d2.bin", "0:/d2.bin", 125, 200, 233, 94, 233, 94}
};
static EColor last_attacks[ATTACKS_NUMBERS] = {0xFF, 0xFF, 0xFF, 0xFF};

// Ajustei para usar as constantes POS_SLOT_...
const uint16_t PLAYER_SLOTS_POS[4][2] = {
    {73,  POS_SLOT_PLAYER_Y_BASE},
    {160, POS_SLOT_PLAYER_Y_BASE},
    {250, POS_SLOT_PLAYER_Y_BASE},
    {340, POS_SLOT_PLAYER_Y_BASE}
};

const uint16_t CPU_SLOTS_POS[4][2] = {
    {165, POS_SLOT_CPU_Y_BASE},
    {203, POS_SLOT_CPU_Y_BASE},
    {238, POS_SLOT_CPU_Y_BASE},
    {275, POS_SLOT_CPU_Y_BASE}
};

// =========================================================
// II. FUNÇÕES AUXILIARES
// =========================================================

void ClearScreen() {
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bg.bin"); 
}

void DrawCachedPlayerIcon(uint16_t x, uint16_t y, EColor element) {
    ILI9488_DrawCachedSprite_Transparent(
        x, y, DIM_ICON_BIG_SIZE, DIM_ICON_BIG_SIZE, PlayerIconCache[element]);
}

void ResetPlayerAttacksCache(void) {
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        last_attacks[i] = 0xFF; 
    }
}
// =========================================================
// III. MENU DIFICULDADE
// =========================================================

void DrawDifficultyMenu(int currentSelection) {
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgd.bin"); 
    
    const DifficultyOptionData_t* option = &DIFFICULTY_DATA[currentSelection];
    
    uint16_t x_sel = option->x_norm - ((option->w_sel - option->w_norm) / 2);
    uint16_t y_sel = option->y_norm - ((option->h_sel - option->h_norm) / 2);

    ILI9488_DrawImage_Transparent(x_sel, y_sel, option->w_sel, option->h_sel, option->sel_path);
}

void DrawSingleDifficultyOption(int index, int isSelected) {
    const DifficultyOptionData_t* option = &DIFFICULTY_DATA[index];

    uint16_t x_sel = option->x_norm - ((option->w_sel - option->w_norm) / 2);
    uint16_t y_sel = option->y_norm - ((option->h_sel - option->h_norm) / 2);

    if (isSelected) {
        ILI9488_DrawImage_Transparent(x_sel, y_sel, option->w_sel, option->h_sel, option->sel_path);
    } else {
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

    // Limpa com borracha
    ILI9488_RestoreRect(POS_MENU_LEFT_X, POS_MENU_SMALL_Y, DIM_MENU_SM_W, DIM_MENU_SM_H, "0:/bgp.bin");
    ILI9488_RestoreRect(POS_MENU_RIGHT_X, POS_MENU_SMALL_Y, DIM_MENU_SM_W, DIM_MENU_SM_H, "0:/bgp.bin");
    ILI9488_RestoreRect(POS_MENU_CENTER_X, POS_MENU_BIG_Y, DIM_PLAYER_BATTLE_W, DIM_PLAYER_BATTLE_H, "0:/bgp.bin");

    // Desenha
    ILI9488_DrawImage_Transparent(POS_MENU_LEFT_X, POS_MENU_SMALL_Y, DIM_MENU_SM_W, DIM_MENU_SM_H, PERSONA_DATA[idx_left].path_menu_sm);
    ILI9488_DrawImage_Transparent(POS_MENU_RIGHT_X, POS_MENU_SMALL_Y, DIM_MENU_SM_W, DIM_MENU_SM_H, PERSONA_DATA[idx_right].path_menu_sm);
    ILI9488_DrawImage_Transparent(POS_MENU_CENTER_X, POS_MENU_BIG_Y, DIM_PLAYER_BATTLE_W, DIM_PLAYER_BATTLE_H, PERSONA_DATA[selectedIndex].path_menu_bg);
}

// =========================================================
// V. TELA DE BATALHA (Setup)
// =========================================================

uint8_t LoadAllIconsToCache(void) {
    FIL file;
    UINT br;
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        if (f_open(&file, ICON_PATHS_BIG[i], FA_READ) != FR_OK) continue; 
        if (f_read(&file, PlayerIconCache[i], DIM_ICON_CACHE_SIZE, &br) != FR_OK || br != DIM_ICON_CACHE_SIZE) {
            f_close(&file);
            return 0; 
        }
        f_close(&file);
    }
    return 1; 
}

void DrawBattleLayout(EDificult difficulty, const EWizard* user, const EWizard* cpu) {
    
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgc.bin"); 

    // Personagens estáticos (usando dimensões IDLE)
    if (user->ePersonaElemental < NUM_ELEMENTS) {
        ILI9488_DrawImage_Transparent(
            10, 80, DIM_PLAYER_BATTLE_W, DIM_PLAYER_BATTLE_H, 
            PERSONA_DATA[user->ePersonaElemental].path_menu_bg
        );
    }

    if (cpu->ePersonaElemental < NUM_ELEMENTS) {
        ILI9488_DrawImage_Transparent(
            200, 60, DIM_CPU_IDLE_W, DIM_CPU_IDLE_H, 
            PERSONA_DATA[cpu->ePersonaElemental].path_cpu
        );
    }

    // Restaura Molduras do Player
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        uint16_t frame_x = PLAYER_SLOTS_POS[i][0] - OFFSET_FRAME_PLAYER_X;
        uint16_t frame_y = PLAYER_SLOTS_POS[i][1] - OFFSET_FRAME_PLAYER_Y;
        ILI9488_RestoreRect(frame_x, frame_y, DIM_FRAME_PLAYER_W, DIM_FRAME_PLAYER_H, "0:/bgc.bin");
    }

    // Restaura Molduras da CPU
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        uint16_t frame_x = CPU_SLOTS_POS[i][0] - OFFSET_FRAME_CPU_X;
        uint16_t frame_y = CPU_SLOTS_POS[i][1] - OFFSET_FRAME_CPU_Y;
        ILI9488_RestoreRect(frame_x, frame_y, DIM_FRAME_CPU_W, DIM_FRAME_CPU_H, "0:/bgc.bin");
    }

    // Ícones de Ataque da CPU
    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        bool draw = false;
        if (difficulty == eDificultEasy) draw = true;
        else if (difficulty == eDificultMedium) draw = (i < 2);

        if (draw && cpu->eAttackSequential[i] < NUM_ELEMENTS) {
            const char* path_cpu_icon = ICON_PATHS_SMALL[cpu->eAttackSequential[i]];
            uint16_t x = CPU_SLOTS_POS[i][0];
            uint16_t y = CPU_SLOTS_POS[i][1];
            ILI9488_DrawImage_Transparent(x, y, DIM_ICON_SMALL_SIZE, DIM_ICON_SMALL_SIZE, path_cpu_icon);
        }
    }
}

void UpdatePlayerAttacks(const EWizard* user) {

    for (int i = 0; i < ATTACKS_NUMBERS; i++) {
        EColor current_element = user->eAttackSequential[i];

        if (current_element != last_attacks[i]) {
            uint16_t icon_x = PLAYER_SLOTS_POS[i][0];
            uint16_t icon_y = PLAYER_SLOTS_POS[i][1];

            uint16_t frame_x = icon_x - OFFSET_FRAME_PLAYER_X;
            uint16_t frame_y = icon_y - OFFSET_FRAME_PLAYER_Y;

            ILI9488_DrawImage_Transparent(frame_x, frame_y, DIM_FRAME_PLAYER_W, DIM_FRAME_PLAYER_H, "0:/b1.bin");

            if (current_element < NUM_ELEMENTS) {
                DrawCachedPlayerIcon(icon_x, icon_y, current_element);
            }
            last_attacks[i] = current_element;
        }
    }
}

// =========================================================
// VI. ANIMAÇÃO DE BATALHA (TELA RESOLUÇÃO)
// =========================================================

void DrawBattleResolutionBg() {
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgAt.bin");
}

void EraseClashIcons(void) {
    // Usa o tamanho do ícone grande para garantir que limpa tudo
    ILI9488_RestoreRect(POS_CLASH_PLAYER_X, POS_CLASH_PLAYER_Y, DIM_ICON_BIG_SIZE, DIM_ICON_BIG_SIZE, "0:/bgAt.bin");
    ILI9488_RestoreRect(POS_CLASH_CPU_X, POS_CLASH_CPU_Y, DIM_ICON_BIG_SIZE, DIM_ICON_BIG_SIZE, "0:/bgAt.bin");
}

void DrawClashIcons(EColor playerColor, EColor cpuColor) {
    DrawCachedPlayerIcon(POS_CLASH_PLAYER_X, POS_CLASH_PLAYER_Y, playerColor);

    if (cpuColor < NUM_ELEMENTS) {
         ILI9488_DrawCachedSprite_Transparent(
             POS_CLASH_CPU_X, POS_CLASH_CPU_Y, 
             DIM_ICON_BIG_SIZE, DIM_ICON_BIG_SIZE, 
             PlayerIconCache[cpuColor] // Reutiliza cache Player (70x70)
         );
    }
}

void UpdateHealthBars(uint8_t currentHpPlayer, uint8_t currentHpCpu) {
    if (currentHpPlayer > 100) currentHpPlayer = 100;
    if (currentHpCpu > 100) currentHpCpu = 100;

    uint16_t wPlayer = (currentHpPlayer * DIM_BAR_W) / 100;
    uint16_t wCpu = (currentHpCpu * DIM_BAR_W) / 100;

    // Fundo cinza
    ILI9488_FillRectangle(POS_BAR_PLAYER_X, POS_BAR_PLAYER_Y, DIM_BAR_W, DIM_BAR_H, 0x3186);
    ILI9488_FillRectangle(POS_BAR_CPU_X, POS_BAR_CPU_Y, DIM_BAR_W, DIM_BAR_H, 0x3186);

    // Vida colorida
    if(wPlayer > 0)
        ILI9488_FillRectangle(POS_BAR_PLAYER_X, POS_BAR_PLAYER_Y, wPlayer, DIM_BAR_H, ILI9488_GREEN);

    if(wCpu > 0)
        ILI9488_FillRectangle(POS_BAR_CPU_X, POS_BAR_CPU_Y, wCpu, DIM_BAR_H, ILI9488_RED);
}

// =========================================================
// EM game_screen.c - SUBSTITUA A FUNÇÃO DrawWizardAction
// =========================================================

// EM game_screen.c

void DrawWizardAction(const EWizard* wiz, uint8_t isPlayer, uint8_t action, const char* bgPath) {
    char filename[30];
    char elemCode[3];
    char suffixStr[5] = ""; 

    uint16_t w = 0, h = 0;
    uint16_t x = 0, y = 0;

    // Mapeia Elemento
    switch(wiz->ePersonaElemental) {
        case eElementalFire: strcpy(elemCode, "Fo"); break;
        case eElementalWater: strcpy(elemCode, "Ag"); break;
        case eElementalEarth: strcpy(elemCode, "Te"); break;
        case eElementalAir: strcpy(elemCode, "Ar"); break;
        case eElementalLight: strcpy(elemCode, "Lu"); break;
        case eElementalShadow: strcpy(elemCode, "So"); break;
        default: return;
    }

    if (isPlayer) {
        // --- PLAYER ---
        
        // Anti-Ghosting: Usamos as dimensões MÁXIMAS possíveis
        // Largura max: Ataque (236)
        // Altura max: Vitória/Derrota (167) -> DIM_PLAYER_RES_H
        
        uint16_t clear_h = DIM_PLAYER_RES_H; 
        uint16_t clear_y = POS_ANCHOR_PLAYER_Y - clear_h;
        
        ILI9488_RestoreRect(POS_ANCHOR_PLAYER_X, clear_y, DIM_PLAYER_ATK_W, clear_h, bgPath);

        // Define Sprite
        if (action == ACTION_ATK) {
            strcpy(suffixStr, "At"); 
            w = DIM_PLAYER_ATK_W; h = DIM_PLAYER_ATK_H;
        } else if (action == ACTION_HURT) {
            strcpy(suffixStr, "Dan"); 
            w = DIM_PLAYER_DMG_W; h = DIM_PLAYER_DMG_H;
        } else if (action == ACTION_WIN) {
             strcpy(suffixStr, "Vir");
             w = DIM_PLAYER_RES_W; h = DIM_PLAYER_RES_H; // 167px
        } else if (action == ACTION_LOSE) {
             strcpy(suffixStr, "Der");
             w = DIM_PLAYER_RES_W; h = DIM_PLAYER_RES_H; // 167px
        } else {
            strcpy(suffixStr, "I"); // Idle
            w = DIM_PLAYER_IDLE_W; h = DIM_PLAYER_IDLE_H;
        }
        if((action == ACTION_WIN) || (action == ACTION_LOSE))
        {
            x = POS_ANCHOR_PLAYER_VIR_X;
            y = POS_ANCHOR_PLAYER_VIR_Y - h;
        }
        else
        {
            x = POS_ANCHOR_PLAYER_X;
            y = POS_ANCHOR_PLAYER_Y - h;
        }
        sprintf(filename, "0:/mg%s%s.bin", elemCode, suffixStr);
    } 
    else {
        // --- CPU ---
        
        // Define Sprite primeiro para saber posição
        if (action == ACTION_ATK) {
            strcpy(suffixStr, "At2"); 
            w = DIM_CPU_ATK_W; h = DIM_CPU_ATK_H;
            x = POS_ANCHOR_CPU_ATK_X;
            y = POS_ANCHOR_CPU_ATK_Y - h;
        } else {
            if (action == ACTION_HURT) {
                strcpy(suffixStr, "Dan2"); 
                w = DIM_CPU_DMG_W; h = DIM_CPU_DMG_H;
                x = POS_ANCHOR_CPU_X;
                y = POS_ANCHOR_CPU_Y - h;
            } else if (action == ACTION_WIN) {
                 strcpy(suffixStr, "Vir2");
                 w = DIM_CPU_DMG_W; h = DIM_CPU_DMG_H; 
                 x = POS_ANCHOR_CPU_VIR_X;
                 y = POS_ANCHOR_CPU_VIR_Y - h;
            } else if (action == ACTION_LOSE) {
                 strcpy(suffixStr, "Der2");
                 w = DIM_CPU_DMG_W; h = DIM_CPU_DMG_H;
                 x = POS_ANCHOR_CPU_VIR_X;
                 y = POS_ANCHOR_CPU_VIR_Y - h;
            } else {
                strcpy(suffixStr, "I2"); // Idle
                w = DIM_CPU_IDLE_W; h = DIM_CPU_IDLE_H;
                x = POS_ANCHOR_CPU_X;
                y = POS_ANCHOR_CPU_Y - h;
            }
        }

        sprintf(filename, "0:/mg%s%s.bin", elemCode, suffixStr);

        // 2. CÁLCULO INTELIGENTE DA ÁREA DE LIMPEZA (Bounding Box)
        
        // Encontra o X mais à esquerda (Início da limpeza)
        uint16_t min_x = (POS_ANCHOR_CPU_X < POS_ANCHOR_CPU_ATK_X) ? POS_ANCHOR_CPU_X : POS_ANCHOR_CPU_ATK_X;
        
        // Encontra o Y mais alto (Topo da limpeza) - Baseado na altura máxima possível (Idle ou Atk)
        uint16_t clean_h = (DIM_CPU_IDLE_H > DIM_CPU_ATK_H) ? DIM_CPU_IDLE_H : DIM_CPU_ATK_H;
        uint16_t clean_y_base = (POS_ANCHOR_CPU_Y > POS_ANCHOR_CPU_ATK_Y) ? POS_ANCHOR_CPU_Y : POS_ANCHOR_CPU_ATK_Y; // Pega a base mais baixa
        uint16_t clean_y = clean_y_base - clean_h; // Sobe a altura máxima

        // CORREÇÃO: Calcula a largura baseada no limite da direita dos sprites, não na soma cega
        // Onde termina o sprite Idle?
        uint16_t max_x_idle = POS_ANCHOR_CPU_X + DIM_CPU_IDLE_W;
        // Onde termina o sprite Ataque?
        uint16_t max_x_atk  = POS_ANCHOR_CPU_ATK_X + DIM_CPU_ATK_W;
        
        // Qual vai mais longe?
        uint16_t limit_x = (max_x_idle > max_x_atk) ? max_x_idle : max_x_atk;
        
        // Largura exata necessária
        uint16_t clean_w = limit_x - min_x;

        // Aplica a borracha calculada
        ILI9488_RestoreRect(min_x, clean_y, clean_w, clean_h, bgPath);
    }

    ILI9488_DrawImage_BIN(x, y, w, h, filename);
}

void DrawEndGameScreen(const EWizard* user, const EWizard* cpu) {
    // 1. Desenha o fundo estático da tela final
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgFim.bin");

    // 2. Verifica condição de vitória do Jogador
    // Assumindo que se o Player tem vida > 0 ele ganhou (ou verifique quem tem mais vida)
    uint8_t playerWins = (user->u8HeartPoints > 0);

    if (playerWins) {
        // --- CENÁRIO DE VITÓRIA ---
        // Sprites
        // Player: Pose de Vitória (Vir)
        DrawWizardAction(user, 1, ACTION_WIN, "0:/bgFim.bin");
        // CPU: Pose de Derrota (Der2)
        DrawWizardAction(cpu, 0, ACTION_LOSE, "0:/bgFim.bin");

        // Mensagens de Vitória
        // "VITORIA" (Grande)
        ILI9488_DrawImage_Transparent(POS_MSG_BIG_X, POS_MSG_BIG_Y, 
                                      DIM_MSG_BIG_W, DIM_MSG_BIG_H, "0:/msV.bin");
        
        // "VOCE VENCEU" (Pequeno) - Ajuste conforme o texto real da imagem
        ILI9488_DrawImage_Transparent(POS_MSG_SMALL_X, POS_MSG_SMALL_Y, 
                                      DIM_MSG_SMALL_W, DIM_MSG_SMALL_H, "0:/msVir.bin");



    } else {
        // --- CENÁRIO DE DERROTA ---
        // Sprites
        // Player: Pose de Derrota (Der)
        DrawWizardAction(user, 1, ACTION_LOSE, "0:/bgFim.bin");
        // CPU: Pose de Vitória (Vir2)
        DrawWizardAction(cpu, 0, ACTION_WIN, "0:/bgFim.bin");
        // Mensagens de Derrota
        // "DERROTA" (Grande)
        ILI9488_DrawImage_Transparent(POS_MSG_BIG_X, POS_MSG_BIG_Y, 
                                      DIM_MSG_BIG_W, DIM_MSG_BIG_H, "0:/msD.bin");
        
        // "VOCE PERDEU" (Pequeno)
        ILI9488_DrawImage_Transparent(POS_MSG_SMALL_X, POS_MSG_SMALL_Y, 
                                      DIM_MSG_SMALL_W, DIM_MSG_SMALL_H, "0:/msDer.bin");


    }
    
}