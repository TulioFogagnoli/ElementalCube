/* vim: set ai et ts:4 sw=4: */
#include "stm32f4xx_hal.h"
#include "ili9488.h"

// Funções estáticas (privadas para este arquivo)
static void ILI9488_Select() {
    HAL_GPIO_WritePin(ILI9488_CS_GPIO_Port, ILI9488_CS_Pin, GPIO_PIN_RESET);
}

void ILI9488_Unselect() {
    HAL_GPIO_WritePin(ILI9488_CS_GPIO_Port, ILI9488_CS_Pin, GPIO_PIN_SET);
}

static void ILI9488_Reset() {
    HAL_GPIO_WritePin(ILI9488_RES_GPIO_Port, ILI9488_RES_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(ILI9488_RES_GPIO_Port, ILI9488_RES_Pin, GPIO_PIN_SET);
    HAL_Delay(120);
}

static void ILI9488_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ILI9488_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

static void ILI9488_WriteData(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_SET);
    // Não otimizar para chunks pequenos como 1 ou 3 bytes
    HAL_SPI_Transmit(&ILI9488_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
}

static void ILI9488_WriteSmallData(uint8_t data) {
    HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(&ILI9488_SPI_PORT, &data, sizeof(data), HAL_MAX_DELAY);
}


static void ILI9488_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Column Address Set
    ILI9488_WriteCommand(0x2A);
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ILI9488_WriteData(data, sizeof(data));
    }
    // Page Address Set
    ILI9488_WriteCommand(0x2B);
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ILI9488_WriteData(data, sizeof(data));
    }
    // Memory Write
    ILI9488_WriteCommand(0x2C);
}

void ILI9488_Init() {
    ILI9488_Select();
    ILI9488_Reset();

    ILI9488_WriteCommand(0xE0); // Positive Gamma Control
    { uint8_t data[] = {0x00, 0x03, 0x09, 0x08, 0x16, 0x0A, 0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08, 0x16, 0x1A, 0x0F}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xE1); // Negative Gamma Control
    { uint8_t data[] = {0x00, 0x16, 0x19, 0x03, 0x0F, 0x05, 0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D, 0x35, 0x37, 0x0F}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xC0); // Power Control 1
    { uint8_t data[] = {0x17, 0x15}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xC1); // Power Control 2
    { uint8_t data[] = {0x41}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xC5); // VCOM Control
    { uint8_t data[] = {0x00, 0x12, 0x80}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0x36); // Memory Access Control
    { uint8_t data[] = {ILI9488_ROTATION}; ILI9488_WriteData(data, sizeof(data)); }
    
    // ============ MUDANÇA IMPORTANTE ANTERIOR ============
    ILI9488_WriteCommand(0x3A); // Interface Pixel Format
    { uint8_t data[] = {0x66}; ILI9488_WriteSmallData(data[0]); } // 0x66 para 18 bits/pixel

    ILI9488_WriteCommand(0xB0); // Interface Mode Control
    { uint8_t data[] = {0x00}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xB1); // Frame Rate Control
    { uint8_t data[] = {0xB0, 0x11}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xB4); // Display Inversion Control
    { uint8_t data[] = {0x02}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xB6); // Display Function Control
    { uint8_t data[] = {0x02, 0x02, 0x3B}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xE9); // Set Image Function
    { uint8_t data[] = {0x00}; ILI9488_WriteData(data, sizeof(data)); }
    ILI9488_WriteCommand(0xF7); // Adjust Control 3
    { uint8_t data[] = {0xA9, 0x51, 0x2C, 0x82}; ILI9488_WriteData(data, sizeof(data)); }
    
    ILI9488_WriteCommand(0x11); // Sleep Out
    HAL_Delay(120);
    ILI9488_WriteCommand(0x29); // Display ON
    ILI9488_Unselect();
}

// ============== NOVA FUNÇÃO DE DESENHO DE PIXEL (3 BYTES) ==============
void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    
    ILI9488_Select();
    ILI9488_SetAddressWindow(x, y, x, y);
    
    // Converte a cor RGB565 (16-bit) para RGB666 (18-bit) enviado em 3 bytes
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;
    
    // Expande para 8 bits
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    uint8_t data[] = {r, g, b};
    ILI9488_WriteData(data, sizeof(data));
    
    ILI9488_Unselect();
}

// ============== NOVA FUNÇÃO DE ESCRITA DE CARACTERE (3 BYTES) ==============
static void ILI9488_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;
    ILI9488_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

    // Converte as cores de background e foreground para 3 bytes uma única vez
    uint8_t color_r = ((color >> 11) << 3);
    uint8_t color_g = ((color >> 5) & 0x3F) << 2;
    uint8_t color_b = (color & 0x1F) << 3;

    uint8_t bgcolor_r = ((bgcolor >> 11) << 3);
    uint8_t bgcolor_g = ((bgcolor >> 5) & 0x3F) << 2;
    uint8_t bgcolor_b = (bgcolor & 0x1F) << 3;
    
    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000) {
                uint8_t pixel[] = {color_r, color_g, color_b};
                ILI9488_WriteData(pixel, sizeof(pixel));
            } else {
                uint8_t pixel[] = {bgcolor_r, bgcolor_g, bgcolor_b};
                ILI9488_WriteData(pixel, sizeof(pixel));
            }
        }
    }
}

// ============== NOVA FUNÇÃO DE PREENCHER RETÂNGULO (3 BYTES) ==============
void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    if((x + w) > ILI9488_WIDTH) w = ILI9488_WIDTH - x;
    if((y + h) > ILI9488_HEIGHT) h = ILI9488_HEIGHT - y;

    ILI9488_Select();
    ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    
    // Converte a cor para 3 bytes
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    
    uint8_t data[] = {r, g, b};
    
    HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_SET);
    for(uint32_t i = 0; i < (h * w); i++) {
        HAL_SPI_Transmit(&ILI9488_SPI_PORT, data, 3, HAL_MAX_DELAY);
    }

    ILI9488_Unselect();
}

// O restante das funções não precisa de alteração pois dependem das que foram corrigidas

void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {
    ILI9488_Select();
    while(*str) {
        if(x + font.width >= ILI9488_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ILI9488_HEIGHT) break;
            if(*str == ' ') {
                str++;
                continue;
            }
        }
        ILI9488_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
    ILI9488_Unselect();
}

void ILI9488_FillScreen(uint16_t color) {
    ILI9488_FillRectangle(0, 0, ILI9488_WIDTH, ILI9488_HEIGHT, color);
}

void ILI9488_DrawImage_RGB565(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    if ((x + w - 1) >= ILI9488_WIDTH) return;
    if ((y + h - 1) >= ILI9488_HEIGHT) return;

    ILI9488_Select();
    ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    // Buffer para os dados de cor convertidos (3 bytes por pixel)
    uint8_t buffer[w * 3]; 

    for (uint16_t i = 0; i < h; i++) {
        for (uint16_t j = 0; j < w; j++) {
            uint16_t color = data[i * w + j];
            
            // Converte a cor RGB565 para RGB666
            uint8_t r = (color >> 11) & 0x1F;
            uint8_t g = (color >> 5) & 0x3F;
            uint8_t b = color & 0x1F;
            
            r = (r << 3) | (r >> 2);
            g = (g << 2) | (g >> 4);
            b = (b << 3) | (b >> 2);
            
            // Armazena no buffer
            buffer[j * 3] = r;
            buffer[j * 3 + 1] = g;
            buffer[j * 3 + 2] = b;
        }
        // Envia uma linha inteira de uma vez
        ILI9488_WriteData(buffer, w * 3);
    }

    ILI9488_Unselect();
}

void ILI9488_DrawImage_RGB666(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* data) {
    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return;
    if ((x + w - 1) >= ILI9488_WIDTH) return;
    if ((y + h - 1) >= ILI9488_HEIGHT) return;

    ILI9488_Select();
    ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    
    // Calcula o tamanho de uma linha da imagem em bytes (largura * 3 bytes por pixel)
    uint32_t line_size_bytes = (uint32_t)w * 3;
    
    // Envia os dados da imagem linha por linha
    for (uint16_t i = 0; i < h; i++) {
        // Calcula o ponteiro para o início da linha atual
        const uint8_t* p_line_data = data + (i * line_size_bytes);
        // Envia a linha inteira para o display
        ILI9488_WriteData((uint8_t*)p_line_data, line_size_bytes);
    }
    
    ILI9488_Unselect();
}

void ILI9488_InvertColors(bool invert) {
    ILI9488_Select();
    ILI9488_WriteCommand(invert ? 0x21 : 0x20); // INVON or INVOFF
    ILI9488_Unselect();
}