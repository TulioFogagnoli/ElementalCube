#include "game_screen.h"
#include "ILI9488.h"
#include "fonts.h"
#include <stdio.h>
#include "main.h"

const DifficultyOptionData_t DIFFICULTY_DATA[] = {
    {"0:/facil.bin", "0:/f2.bin", 110, 20, 265, 89, 290, 100},
    {"0:/medio.bin", "0:/m2.bin", 110, 120, 265, 89, 290, 100},
    {"0:/dificil.bin", "0:/d2.bin", 110, 220, 265, 89, 290, 100}
};

void ClearScreen() {
    ILI9488_FillRectangle(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT, ILI9488_BLACK);
}

void DrawMenu(const char* title, const char** options, int numOptions, int currentSelection) {
    char buffer[30];

    // Desenha o título com a fonte maior, mais abaixo no ecrã
    sprintf(buffer, "%s", title);
    ILI9488_WriteString(0, 0, buffer, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);

    // Desenha as opções com mais espaçamento vertical
    for (int i = 0; i < numOptions; i++) {
        uint16_t color = (i == currentSelection) ? ILI9488_YELLOW : ILI9488_WHITE;
        sprintf(buffer, "%s %s", (i == currentSelection) ? ">" : " ", options[i]);
        // Aumenta o espaçamento entre as linhas (de 15 para 25)
        ILI9488_WriteString(0, 30 + (i * 20), buffer, Font_7x10, color, ILI9488_BLACK);
    }
}

void DrawDifficultyMenu(int currentSelection) {
    char buffer[30];
    
    // Desenha o título
    //ILI9488_WriteString(20, 30, "Selecione Dificuldade", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);

    // Desenha o background do menu (Se não for dinâmico, pode ser movido para a transição de estado)
    ILI9488_DrawImage_BIN(0, 0, 480, 320, "0:/bgd.bin"); 
    
    // Desenha as opções
    for (int i = 0; i < MENU_OPTIONS_DIFFICULTY; i++) {
        const DifficultyOptionData_t* option = &DIFFICULTY_DATA[i];
        
        uint16_t w_to_draw, h_to_draw;
        uint16_t x_to_draw, y_to_draw;
        const char* path_to_draw;

        if (i == currentSelection) {
            // Opção Selecionada (Versão maior)
            path_to_draw = option->sel_path;
            w_to_draw = option->w_sel;
            h_to_draw = option->h_sel;
            
            // Centraliza o sprite maior no espaço X/Y do sprite normal:
            // X_inicial - (metade da diferença de largura)
            // Y_inicial - (metade da diferença de altura)
            x_to_draw = option->x_norm - ((option->w_sel - option->w_norm) / 2);
            y_to_draw = option->y_norm - ((option->h_sel - option->h_norm) / 2);

        } else {
            // Opção Normal (Versão padrão)
            path_to_draw = option->norm_path;
            w_to_draw = option->w_norm;
            h_to_draw = option->h_norm;
            x_to_draw = option->x_norm;
            y_to_draw = option->y_norm;
        }

        // Desenha a imagem usando a função transparente (ILI9488_DrawImage_Transparent)
        if (!ILI9488_DrawImage_Transparent(x_to_draw, y_to_draw, w_to_draw, h_to_draw, path_to_draw)) {
             // Fallback de erro
             sprintf(buffer, "Erro: %s", path_to_draw);
             ILI9488_WriteString(option->x_norm, option->y_norm, buffer, Font_7x10, ILI9488_RED, ILI9488_BLACK);
        }
    }
}