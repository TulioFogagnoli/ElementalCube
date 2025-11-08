/* vim: set ai et ts=4 sw=4: */
#ifndef __ILI9488_H__
#define __ILI9488_H__

#include "fonts.h"
#include <stdbool.h>
#include "main.h"

// Definições do controlador de tela
#define ILI9488_WIDTH       480
#define ILI9488_HEIGHT      320

// Comandos de orientação da tela (MADCTL)
#define ILI9488_MADCTL_MY  0x80
#define ILI9488_MADCTL_MX  0x40
#define ILI9488_MADCTL_MV  0x20
#define ILI9488_MADCTL_ML  0x10
#define ILI9488_MADCTL_BGR 0x08
#define ILI9488_MADCTL_MH  0x04

// Configuração da rotação para modo paisagem
#define ILI9488_ROTATION (ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR)


#define ILI9488_SPI_PORT hspi1
extern SPI_HandleTypeDef ILI9488_SPI_PORT;

#define ILI9488_RES_Pin         LCD_RST_Pin
#define ILI9488_RES_GPIO_Port   LCD_RST_GPIO_Port
#define ILI9488_CS_Pin          LCD_CS_Pin
#define ILI9488_CS_GPIO_Port    LCD_CS_GPIO_Port
#define ILI9488_DC_Pin          LCD_DC_Pin
#define ILI9488_DC_GPIO_Port    LCD_DC_GPIO_Port
#define ILI9488_LED_Pin         LCD_LED_Pin
#define ILI9488_LED_GPIO_Port   LCD_LED_GPIO_Port
/**************************************************************************/

// Definições de Cores (16-bit, formato 565)
#define	ILI9488_BLACK   0x0000
#define	ILI9488_BLUE    0x001F
#define	ILI9488_RED     0xF800
#define	ILI9488_GREEN   0x07E0
#define ILI9488_CYAN    0x07FF
#define ILI9488_MAGENTA 0xF81F
#define ILI9488_YELLOW  0xFFE0
#define ILI9488_WHITE   0xFFFF
#define ILI9488_BROWN   0x51E0
#define ILI9488_GRAY    0x630C

// Protótipos das Funções
void ILI9488_Unselect();
void ILI9488_Init(void);
void ILI9488_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9488_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9488_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9488_FillScreen(uint16_t color);
void ILI9488_DrawImage_RGB666(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* data);
void ILI9488_InvertColors(bool invert);
uint8_t ILI9488_DrawImage_BIN(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* filepath);
#endif // __ILI9488_H__