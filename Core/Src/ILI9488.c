/* vim: set ai et ts:4 sw=4: */
#include "stm32f4xx_hal.h"
#include "ili9488.h"

#include "ff.h"      // Funções principais do FatFs (f_open, f_read, etc.)
#include "fatfs.h"   // Para definições do FatFs (opcional se ff.h já incluir)
#include <stdlib.h>
#include "FreeRTOS.h" 
#include "task.h"     
extern osSemaphoreId spiTxSemaHandle; 


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

static void ILI9488_WriteDataDMA(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_SET);

    osSemaphoreWait(spiTxSemaHandle, 0);
    // Inicia a transferência
    HAL_SPI_Transmit_DMA(&ILI9488_SPI_PORT, buff, buff_size);
    
    // Bloqueia a DisplayTask até que o DMA termine e o Semáforo seja liberado pelo Callback
    osSemaphoreWait(spiTxSemaHandle, osWaitForever);
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
        ILI9488_WriteDataDMA((uint8_t*)p_line_data, line_size_bytes);
    }
    
    ILI9488_Unselect();
}

void ILI9488_InvertColors(bool invert) {
    ILI9488_Select();
    ILI9488_WriteCommand(invert ? 0x21 : 0x20); // INVON or INVOFF
    ILI9488_Unselect();
}

/**
  * @brief  Desenha uma imagem .bin (RGB666) lida do cartão SD.
  * @param  x: Posição X inicial.
  * @param  y: Posição Y inicial.
  * @param  w: Largura da imagem.
  * @param  h: Altura da imagem.
  * @param  filepath: Caminho completo para o arquivo .bin no SD (ex: "0:/imagem.bin").
  * @note   O arquivo .bin DEVE conter dados brutos de pixel no formato RGB666 (3 bytes por pixel).
  * @note   Esta função aloca um buffer de linha no heap (malloc).
  * @retval 1 em sucesso, 0 em falha (falha ao abrir, ler ou alocar memória).
  */
uint8_t ILI9488_DrawImage_BIN(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* filepath) {
    FIL file;
    FRESULT res;
    UINT br = 0; // Bytes lidos

    // 1. Verifica limites
    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return 0;
    if ((x + w - 1) >= ILI9488_WIDTH) w = ILI9488_WIDTH - x;
    if ((y + h - 1) >= ILI9488_HEIGHT) h = ILI9488_HEIGHT - y;

    // 2. Calcula tamanho da linha (RGB666 = 3 bytes/pixel)
    uint32_t line_size_bytes = (uint32_t)w * 3;

    // 3. Aloca um buffer na STACK para a linha
    //    (Requer Stack_Size >= 0x1000 (4KB) em startup_stm32f407vetx.s)
    uint8_t line_buffer[line_size_bytes];

    // 4. Abre o arquivo
    res = f_open(&file, filepath, FA_READ);
    if (res != FR_OK) {
        return 0; // Falha ao abrir o arquivo
    }

    // 5. Prepara o display
    ILI9488_Select();
    ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    //

    // 6. Loop de leitura e desenho
    for (uint16_t i = 0; i < h; i++) {
        // Lê uma linha (RGB666) do arquivo direto para o buffer
        res = f_read(&file, line_buffer, line_size_bytes, &br);
        if (res != FR_OK || br != line_size_bytes) {
            break; // Fim de arquivo ou erro de leitura
        }

        // NÃO HÁ CONVERSÃO
        // Os dados em line_buffer já estão no formato que o display espera.
        
        // Envia a linha (RGB666) para o display
        ILI9488_WriteDataDMA(line_buffer, line_size_bytes);
    }

    // 7. Limpeza
    ILI9488_Unselect();
    f_close(&file);

    // 8. Retorno
    if (br != line_size_bytes && h > 0) {
         return 0; // Leitura falhou
    }

    return 1; // Sucesso
}

uint8_t ILI9488_DrawImage_Transparent(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* filepath) {
FIL file;
    FRESULT res;
    UINT br = 0;

    // 1. Verificações de limites
    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return 0;
    if ((x + w) > ILI9488_WIDTH) w = ILI9488_WIDTH - x;
    if ((y + h) > ILI9488_HEIGHT) h = ILI9488_HEIGHT - y;

    // 2. Prepara Buffer de Linha
    uint32_t line_size_bytes = (uint32_t)w * 3;
    // VLA (Variable Length Array) na stack. Se der estouro, use static ou malloc.
    uint8_t line_buffer[line_size_bytes]; 

    // 3. Abre Arquivo
    res = f_open(&file, filepath, FA_READ);
    if (res != FR_OK) return 0;

    ILI9488_Select();

    // 4. Loop Linha por Linha (Y)
    for (uint16_t i = 0; i < h; i++) {
        
        // Lê a linha inteira do SD para o buffer
        res = f_read(&file, line_buffer, line_size_bytes, &br);
        if (res != FR_OK || br < line_size_bytes) break;

        // Processa a linha horizontalmente (X) procurando BLOCOS de pixels opacos
        for (uint16_t j = 0; j < w; j++) {
            
            uint8_t* p_pixel = &line_buffer[j * 3];
            
            // Verifica se é a Cor Chave (Magenta: 0xFF, 0x00, 0xFF)
            if (p_pixel[0] == ILI9488_COLOR_KEY_R && 
                p_pixel[1] == ILI9488_COLOR_KEY_G && 
                p_pixel[2] == ILI9488_COLOR_KEY_B) {
                // É transparente: Pula este pixel e continua o loop
                continue; 
            } 
            
            // SE É OPACO: Calcula o tamanho do bloco contínuo (run_len)
            uint16_t run_len = 1;
            while ((j + run_len) < w) {
                uint8_t* p_next = &line_buffer[(j + run_len) * 3];
                
                // Se encontrar um pixel transparente, o bloco acaba.
                if (p_next[0] == ILI9488_COLOR_KEY_R && 
                    p_next[1] == ILI9488_COLOR_KEY_G && 
                    p_next[2] == ILI9488_COLOR_KEY_B) {
                    break;
                }
                run_len++;
            }

            // DESENHA O BLOCO OPACO
            // 1. Define a janela para o tamanho exato do bloco (run_len pixels)
            ILI9488_SetAddressWindow(x + j, y + i, x + j + run_len - 1, y + i);
            
            // 2. Envia os dados usando sua função DMA encapsulada
            // Passamos o ponteiro do início do bloco e o tamanho total em bytes
            ILI9488_WriteDataDMA(p_pixel, run_len * 3);
            
            // 3. Avança o índice 'j' para pular os pixels que já enviamos neste bloco
            j += (run_len - 1); 
        }
    }

    ILI9488_Unselect();
    f_close(&file);
    return 1;
}

// ILI9488.c

// Restaura uma área retangular da tela usando pixels do arquivo de background (480x320)
uint8_t ILI9488_RestoreRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* filepath) {
    FIL file;
    FRESULT res;
    UINT br;

    // 1. Verifica limites
    if ((x >= ILI9488_WIDTH) || (y >= ILI9488_HEIGHT)) return 0;
    if ((x + w) > ILI9488_WIDTH) w = ILI9488_WIDTH - x;
    if ((y + h) > ILI9488_HEIGHT) h = ILI9488_HEIGHT - y;

    // 2. Abre o arquivo de background
    res = f_open(&file, filepath, FA_READ);
    if (res != FR_OK) return 0;

    // 3. Preparações
    uint32_t full_image_width = 480; // Largura original do bgm.bin
    uint32_t bytes_per_pixel = 3;
    
    // Buffer para ler uma linha do RECORTE
    uint32_t line_size = w * bytes_per_pixel;
    uint8_t line_buffer[line_size];

    ILI9488_Select();

    // Define a janela de escrita no display para o tamanho do recorte
    ILI9488_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    // Envia o comando de escrita de memória UMA vez antes do loop
    HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_RESET);
    uint8_t cmd = 0x2C;
    HAL_SPI_Transmit(&ILI9488_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ILI9488_DC_GPIO_Port, ILI9488_DC_Pin, GPIO_PIN_SET);

    // 4. Loop Linha por Linha
    for (uint16_t i = 0; i < h; i++) {
        
        // Calcula onde começa esta linha específica dentro do arquivo bgm.bin
        // Offset = (Linha Atual Y * Largura Total * 3) + (Coluna Inicial X * 3)
        uint32_t file_offset = ((y + i) * full_image_width * bytes_per_pixel) + (x * bytes_per_pixel);
        
        // Move o cursor do arquivo para o ponto exato
        f_lseek(&file, file_offset);
        
        // Lê apenas a largura do recorte
        res = f_read(&file, line_buffer, line_size, &br);
        if (res != FR_OK || br < line_size) break;

        // Envia para o display (pode usar a função DMA ou Síncrona aqui)
        // Como estamos com f_lseek, DMA pode não ser tão vantajoso, mas vamos usar para manter o padrão
        // Se der erro, troque por HAL_SPI_Transmit
        osSemaphoreWait(spiTxSemaHandle, 0); // Limpa semáforo
        HAL_SPI_Transmit_DMA(&ILI9488_SPI_PORT, line_buffer, line_size);
        osSemaphoreWait(spiTxSemaHandle, osWaitForever); // Espera terminar
    }

    ILI9488_Unselect();
    f_close(&file);
    return 1;
}