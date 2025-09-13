#include "game_screen.h"
#include "ili9341.h"
#include "fonts.h"
#include "stdint.h"
#include <stdio.h>

void ClearScreen() {
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, ILI9341_BLACK);
}

void DrawMenu(const char* title, const char** options, int numOptions, int currentSelection) {
    char buffer[30];

    // Desenha o título com a fonte maior, mais abaixo no ecrã
    sprintf(buffer, "%s", title);
    ILI9341_WriteString(10, 50, buffer, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);

    // Desenha as opções com mais espaçamento vertical
    for (int i = 0; i < numOptions; i++) {
        uint16_t color = (i == currentSelection) ? ILI9341_YELLOW : ILI9341_WHITE;
        sprintf(buffer, "%s %s", (i == currentSelection) ? ">" : " ", options[i]);
        // Aumenta o espaçamento entre as linhas (de 15 para 25)
        ILI9341_WriteString(10, 100 + (i * 25), buffer, Font_11x18, color, ILI9341_BLACK);
    }
}