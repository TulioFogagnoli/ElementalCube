#include "game_screen.h"
#include "st7789.h"
#include "stdint.h"

void ClearScreen() {
    ST7789_FillRectangle(0, 0, 240, 240, ST7789_BLACK);
}

void DrawMenu(const char* title, const char** options, int numOptions, int currentSelection) {
    char buffer[30];
    sprintf(buffer, "%s", title);
    ST7789_DrawText(10, 10, buffer, ST7789_WHITE, ST7789_BLACK, ST7789_SIZE);

    for (int i = 0; i < numOptions; i++) {
        uint16_t color = (i == currentSelection) ? ST7789_YELLOW : ST7789_WHITE;
        sprintf(buffer, "%s %s", (i == currentSelection) ? ">" : " ", options[i]);
        ST7789_DrawText(10, 40 + (i * 20), buffer, color, ST7789_BLACK, ST7789_SIZE);
    }
}