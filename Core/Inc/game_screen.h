#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include <stdint.h>

// Definição da estrutura para agrupar todos os dados de uma opção
typedef struct {
    const char* norm_path; // Caminho da imagem normal
    const char* sel_path;  // Caminho da imagem selecionada
    uint16_t x_norm;       // Posição X (Normal)
    uint16_t y_norm;       // Posição Y (Normal)
    uint16_t w_norm;       // Largura da imagem normal (Ex: 300)
    uint16_t h_norm;       // Altura da imagem normal (Ex: 100)
    uint16_t w_sel;        // Largura da imagem selecionada (Ex: 330)
    uint16_t h_sel;        // Altura da imagem selecionada (Ex: 110)
} DifficultyOptionData_t;

// Dados para o menu de Dificuldade
extern const DifficultyOptionData_t DIFFICULTY_DATA[];



void ClearScreen();
void DrawMenu(const char* title, const char** options, int numOptions, int currentSelection);
void DrawDifficultyMenu(int currentSelection);
void DrawSingleDifficultyOption(int index, int isSelected);
#endif //GAME_SCREEN_H