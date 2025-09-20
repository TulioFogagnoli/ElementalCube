#include "game_screen.h"
#include "ILI9488.h"
#include "fonts.h"
#include "stdint.h"
#include <stdio.h>

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