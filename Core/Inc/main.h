/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA
#define LCD_SCK_Pin GPIO_PIN_5
#define LCD_SCK_GPIO_Port GPIOA
#define LCD_MISO_Pin GPIO_PIN_6
#define LCD_MISO_GPIO_Port GPIOA
#define LCD_MOSI_Pin GPIO_PIN_7
#define LCD_MOSI_GPIO_Port GPIOA
#define LCD_RST_Pin GPIO_PIN_0
#define LCD_RST_GPIO_Port GPIOB
#define LCD_DC_Pin GPIO_PIN_1
#define LCD_DC_GPIO_Port GPIOB
#define LCD_LED_Pin GPIO_PIN_7
#define LCD_LED_GPIO_Port GPIOC
#define BTN_CONFIRM_PORT  GPIOD
#define BTN_CONFIRM_PIN   GPIO_PIN_1
#define BTN_BACK_PORT     GPIOD
#define BTN_BACK_PIN      GPIO_PIN_3
#define BTN_UP_PORT       GPIOD
#define BTN_UP_PIN        GPIO_PIN_5
#define BTN_DOWN_PORT     GPIOD
#define BTN_DOWN_PIN      GPIO_PIN_7


/* USER CODE BEGIN Private defines */
#define FALSE 0
#define TRUE 1

#define UP_KEY '8'
#define DOWN_KEY '2'
#define CONFIRM_KEY '*'
#define BACK_KEY '#'
#define FIRE_KEY 'A'
#define WATER_KEY 'B'
#define AIR_KEY 'C'
#define EARTH_KEY 'D'
#define NONE_KEY '\0'

#define MENU_OPTIONS_DIFFICULTY 3
#define MENU_OPTIONS_PERSONA 5

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
