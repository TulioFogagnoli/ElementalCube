#include "st7789.h"
#include "main.h" // Para acessar os defines dos pinos e a instância do SPI
#include "font.h" 

// Associe a instância do SPI gerada pelo CubeIDE
extern SPI_HandleTypeDef hspi1;
#define ST7789_SPI_PORT hspi1

// Mapeamento dos pinos (usando os User Labels do CubeIDE)
#define LCD_CS_PORT   LCD_CS_GPIO_Port
#define LCD_CS_PIN    LCD_CS_Pin
#define LCD_DC_PORT   LCD_DC_GPIO_Port
#define LCD_DC_PIN    LCD_DC_Pin
#define LCD_RST_PORT  LCD_RST_GPIO_Port
#define LCD_RST_PIN   LCD_RST_Pin

// Funções privadas (auxiliares)
static void ST7789_Select(void) {
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
}

static void ST7789_Unselect(void) {
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

static void ST7789_Reset(void) {
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(5);
}

static void ST7789_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET); // Modo Comando
    ST7789_Select();
    HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
    ST7789_Unselect();
}

static void ST7789_WriteData(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET); // Modo Dado
    ST7789_Select();
    HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
    ST7789_Unselect();
}

// Define a "janela" de memória onde os pixels serão escritos
static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[4];

    // Column Address Set
    ST7789_WriteCommand(0x2A);
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    ST7789_WriteData(data, sizeof(data));

    // Row Address Set
    ST7789_WriteCommand(0x2B);
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    ST7789_WriteData(data, sizeof(data));

    // Write to RAM
    ST7789_WriteCommand(0x2C);
}

// Função de inicialização com a sequência de comandos para o ST7789
void ST7789_Init(void) {
    ST7789_Reset();

    ST7789_WriteCommand(0x11); // Sleep Out
    HAL_Delay(120);

    ST7789_WriteCommand(0x36); // Memory Data Access Control
    uint8_t madctl = 0x00;
    ST7789_WriteData(&madctl, 1);

    ST7789_WriteCommand(0x3A); // Interface Pixel Format
    uint8_t pixfmt = 0x55; // 16 bits/pixel
    ST7789_WriteData(&pixfmt, 1);

    ST7789_WriteCommand(0x21); // Display Inversion On

    ST7789_WriteCommand(0x13); // Normal Display Mode On

    ST7789_WriteCommand(0x29); // Display On
    HAL_Delay(120);
}

// Funções públicas (implementação)
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;

    ST7789_SetAddressWindow(x, y, x, y);
    uint8_t data[] = { (color >> 8) & 0xFF, color & 0xFF };
    ST7789_WriteData(data, sizeof(data));
}

void ST7789_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;
    if ((x + w - 1) >= ST7789_WIDTH) w = ST7789_WIDTH - x;
    if ((y + h - 1) >= ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint8_t data[] = { (color >> 8) & 0xFF, color & 0xFF };
    HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET); // Data mode
    ST7789_Select();

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            HAL_SPI_Transmit(&ST7789_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
        }
    }
    ST7789_Unselect();
}

void ST7789_FillScreen(uint16_t color) {
    ST7789_FillRectangle(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

/**
 * @brief Desenha um único caractere no display.
 * @param x Coordenada X do canto superior esquerdo.
 * @param y Coordenada Y do canto superior esquerdo.
 * @param ch O caractere a ser desenhado.
 * @param foreground Cor do caractere.
 * @param background Cor do fundo.
 */
void ST7789_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t foreground, uint16_t background) {
    // Verifica se o caractere caberá na tela
    if (x + FONT_WIDTH > ST7789_WIDTH || y + FONT_HEIGHT > ST7789_HEIGHT) {
        return;
    }

    // Calcula o índice inicial do caractere no array da fonte
    // A fonte começa no caractere de espaço ' ' (ASCII 32)
    uint32_t font_idx = (ch - ' ') * FONT_HEIGHT;

    // Loop para cada linha de pixels do caractere (altura, eixo Y)
    for (int i = 0; i < FONT_HEIGHT; i++) {
        
        // Pega o byte que representa a linha horizontal de pixels
        uint8_t line_data = font[font_idx + i];

        // Loop para cada pixel dentro da linha (largura, eixo X)
        for (int j = 0; j < FONT_WIDTH; j++) {
            
            // Verifica se o bit correspondente ao pixel está ligado
            if ((line_data >> j) & 1) {
                // Se o bit é 1, desenha com a cor da frente (foreground)
                // AQUI ESTÁ A LÓGICA CRÍTICA:
                // A coordenada X da tela é modificada pelo loop da largura (j)
                // A coordenada Y da tela é modificada pelo loop da altura (i)
                ST7789_DrawPixel(x + i, y + j, foreground);
            } else {
                // Se o bit é 0, desenha com a cor de fundo (background)
                ST7789_DrawPixel(x + i, y + j, background);
            }
        }
    }
}

/**
 * @brief Desenha uma string (texto) no display.
 * @param x Coordenada X do início do texto.
 * @param y Coordenada Y do início do texto.
 * @param str Ponteiro para a string a ser desenhada.
 * @param foreground Cor do texto.
 * @param background Cor do fundo.
 */
void ST7789_DrawText(uint16_t x, uint16_t y, const char* str, uint16_t foreground, uint16_t background) {
    uint16_t current_x = x;

    while (*str) {
        // Verifica se o próximo caractere cabe na tela
        if (current_x + FONT_WIDTH > ST7789_WIDTH) {
            // Aqui você poderia implementar quebra de linha, se quisesse.
            // Por enquanto, apenas paramos de desenhar.
            break; 
        }

        ST7789_DrawChar(current_x, y, *str, foreground, background);
        current_x += FONT_WIDTH; // Avança para a posição do próximo caractere
        str++; // Avança para o próximo caractere na string
    }
}