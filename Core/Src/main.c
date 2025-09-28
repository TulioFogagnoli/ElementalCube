/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ILI9488.h"
#include "TCS3472.h"
#include "TCA9548A.h"

#include "Logo300.h"
#include "background.h"

#include "fonts.h"
#include "keypad.h"

#include "game_fsm.h"
#include "game_types.h"
#include "game_screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart4;

osThreadId inputHalTaskHandle;
osThreadId gameTaskHandle;
osThreadId displayTaskHandle; 
osMutexId gameMutexHandle;   

volatile char keyPressed = '\0';
volatile EGameStates eCurrentState = eInitGame;
volatile int selectedOption = 0;
volatile uint8_t u8CleanScreen = TRUE;
volatile EDificult selectedDifficulty;
volatile EWizard eUserPlayer;
volatile EWizard eCpuPlayer;
volatile TCS3472_Data colorData[4];
volatile uint8_t u8ContAttack = 0;
const char* difficultyOptions[MENU_OPTIONS_DIFFICULTY] = {"Facil", "Medio", "Dificil"};
const char* personaOptions[MENU_OPTIONS_PERSONA] = {"Mago de Fogo", "Mago de Agua", "Mago de Terra", "Mago de Ar", "Mago da Luz"};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);
void StartInputHalTask(void const * argument);
void StartGameTask(void const * argument);
void StartDisplayTask(void const * argument);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  //--------------------------------------------------------------------
  // ETAPA DE INICIALIZAÇÃO
  //--------------------------------------------------------------------

  // 1. Inicializa o display. Ele usará a velocidade alta do SPI configurada
  //    no MX_SPI1_Init(), o que é ótimo para performance gráfica.
  ILI9488_Init();

  // 2. Acende o backlight do display.
  //    (Assumindo que seu pino é o LCD_LED_Pin, como no seu MX_GPIO_Init)
  HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_SET);

  // 3. Prepara a tela para o usuário com uma mensagem de boas-vindas.
  ILI9488_FillScreen(ILI9488_BLACK);
  
  char buffer[40];

  // 1. Verifica se o multiplexador TCA9548A está respondendo
  ILI9488_WriteString(10, 30, "Verificando MUX...", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
  if (HAL_I2C_IsDeviceReady(&hi2c1, MUX_ADDR, 2, 100) == HAL_OK)
  {
    ILI9488_WriteString(10, 50, "Multiplexador OK!", Font_7x10, ILI9488_GREEN, ILI9488_BLACK);
  }
  else
  {
    ILI9488_WriteString(10, 50, "Falha no Multiplexador!", Font_7x10, ILI9488_RED, ILI9488_BLACK);
    // Trava aqui se o MUX falhar, pois nada mais vai funcionar
    while (1);
  }
  HAL_Delay(1000);


  // 2. Inicializa e verifica cada um dos 4 sensores de cor
  for (int i = 0; i < 4; i++)
  {
    sprintf(buffer, "Iniciando Sensor %d...", i + 1);
    ILI9488_WriteString(10, 80 + (i * 30), buffer, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
    HAL_Delay(100);

    if (TCS3472_Init(&hi2c1, i))
    {
      sprintf(buffer, "Sensor de Cor %d OK!", i + 1);
      ILI9488_WriteString(10, 95 + (i * 30), buffer, Font_7x10, ILI9488_GREEN, ILI9488_BLACK);
    }
    else
    {
      sprintf(buffer, "Erro no Sensor de Cor %d!", i + 1);
      ILI9488_WriteString(10, 95 + (i * 30), buffer, Font_7x10, ILI9488_RED, ILI9488_BLACK);
    }
    HAL_Delay(250); // Aumenta o tempo para podermos ler
  }

  ILI9488_WriteString(20, 120, "Sistema Iniciado!", Font_7x10, ILI9488_GREEN, ILI9488_BLACK);
  HAL_Delay(2000); // Uma pequena pausa para o usuário ler a mensagem.

  // 4. Limpa a tela para começar a desenhar.
  ILI9488_FillScreen(ILI9488_BLACK);

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osMutexDef(gameMutex);
  gameMutexHandle = osMutexCreate(osMutex(gameMutex));

  osThreadDef(initPutHalTask, StartInputHalTask, osPriorityNormal, 0, 128);
  inputHalTaskHandle = osThreadCreate(osThread(initPutHalTask), NULL);

  osThreadDef(gameTask, StartGameTask, osPriorityNormal, 0, 256);
  gameTaskHandle = osThreadCreate(osThread(gameTask), NULL);

  osThreadDef(displayTask, StartDisplayTask, osPriorityNormal, 0, 256);
  displayTaskHandle = osThreadCreate(osThread(displayTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_RST_Pin|LCD_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, C1_Pin|C2_Pin|C3_Pin|C4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RST_Pin LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_LED_Pin */
  GPIO_InitStruct.Pin = LCD_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : C1_Pin C2_Pin C3_Pin C4_Pin */
  GPIO_InitStruct.Pin = C1_Pin|C2_Pin|C3_Pin|C4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : R1_Pin R2_Pin R3_Pin R4_Pin */
  GPIO_InitStruct.Pin = R1_Pin|R2_Pin|R3_Pin|R4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

void StartInputHalTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  char cCurrent;
  /* Infinite loop */
  for(;;) // O loop infinito de uma tarefa RTOS é "for(;;)"
  {
    cCurrent = KEYPAD_Scan(); // Escaneia o teclado
    if(cCurrent != '\0')
    {
      keyPressed = cCurrent;
    }
    for (int i = 0; i < 4; i++) {
        TCS3472_ReadData(&hi2c1, i, &colorData[i]);
    }
    osDelay(50);
  }
}

void StartGameTask(void const * argument)
{
  char cLocalKeyPressed;
  for(;;)
  {
    cLocalKeyPressed = NONE_KEY;
    
    osMutexWait(gameMutexHandle, osWaitForever);
    if (keyPressed != NONE_KEY) {
      cLocalKeyPressed = keyPressed;
      keyPressed = NONE_KEY; 
    }
    osMutexRelease(gameMutexHandle);
    
    if (cLocalKeyPressed != NONE_KEY)
    {
      osMutexWait(gameMutexHandle, osWaitForever);
      switch(eCurrentState)
      {
        case eInitGame:
        {
          eUserPlayer.u8HeartPoints = 100;
          eCpuPlayer.u8HeartPoints = 100;
          if(cLocalKeyPressed == CONFIRM_KEY)
          {
            eCurrentState = eDificultSelect;
            selectedOption = 0;
            u8CleanScreen = TRUE;
            }
          break;
        }
        case eDificultSelect:
        {
          switch(cLocalKeyPressed)
          {
            case UP_KEY:
            {
              selectedOption = (selectedOption < MENU_OPTIONS_DIFFICULTY - 1) ? selectedOption + 1 : 0;
              u8CleanScreen = TRUE;
              break;
  }
            case DOWN_KEY:
            {
              selectedOption = (selectedOption > 0) ? selectedOption - 1 : MENU_OPTIONS_DIFFICULTY - 1;
              u8CleanScreen = TRUE;
              break;
            }
            case BACK_KEY:
            {
              u8CleanScreen = TRUE;
              eCurrentState = eInitGame;
              break;
            }
            case CONFIRM_KEY:
            {
              selectedDifficulty = (EDificult)selectedOption;
              eCurrentState = ePersonaSelect;
              selectedOption = FALSE; // Reseta para o próximo menu
              u8CleanScreen = TRUE;
              break;
            }
            default:
            {
              break;
            }
          }
          break;
        }
        case ePersonaSelect:
        {
          switch(cLocalKeyPressed)
          {
            case UP_KEY:
            {                  
              selectedOption = (selectedOption < MENU_OPTIONS_PERSONA - 1) ? selectedOption + 1 : 0;
              u8CleanScreen = TRUE;
              break;
            }
            case DOWN_KEY:
            {
              selectedOption = (selectedOption > 0) ? selectedOption - 1 : MENU_OPTIONS_PERSONA - 1;
              u8CleanScreen = TRUE;
              break;
            }
            case BACK_KEY:
            {
              u8CleanScreen = TRUE;
              eCurrentState = eDificultSelect;
              break;
            }
            case CONFIRM_KEY:
            {
              eUserPlayer.ePersonaElemental = (EElemental)selectedOption;
              eCurrentState = eBattleInit;
              u8CleanScreen = TRUE;
              u8ContAttack = 0;     // Zera o contador de ataques
              selectedOption = 0;   // Inicia com a primeira opção (Fogo) pré-selecionada

              memset((void*)eUserPlayer.eAttackSequential, 0, sizeof(eUserPlayer.eAttackSequential));
              memset((void*)eCpuPlayer.eAttackSequential, 0, sizeof(eCpuPlayer.eAttackSequential));

              srand(HAL_GetTick()); 
              eCpuPlayer.ePersonaElemental = (rand() % 6);
              for(uint8_t u8Idx = 0; u8Idx < ATTACKS_NUMBERS; u8Idx++)
              {
                eCpuPlayer.eAttackSequential[u8Idx] = (EColor)(rand() % 6); 
              }
              break;
            }
            default:
            {
              break;
            }
          }
          break;
        }
        case eBattleInit:
        {
          // A lógica de ataque agora só depende das teclas CONFIRM e BACK
          switch (cLocalKeyPressed)
          {
            case CONFIRM_KEY:
            {
              // Lê a cor de cada um dos 4 sensores e define a sequência de ataque
              for (int i = 0; i < ATTACKS_NUMBERS; i++)
              {
                  eUserPlayer.eAttackSequential[i] = TCS3472_DetectColor(colorData[i]);
              }

              // Prepara e inicia a batalha
              vInitBattle(&eUserPlayer, &eCpuPlayer);
              eCurrentState = ePlayerTurn;
              u8CleanScreen = TRUE;
              break;
            } 
            case BACK_KEY:
            {
              eCurrentState = ePersonaSelect;
              u8CleanScreen = TRUE;
              break;
            }
            default:
              // Ignora outras teclas, pois a tela se atualiza sozinha
              break;
          }
          break;
        }
        case ePlayerTurn:
        {
          if(cLocalKeyPressed == CONFIRM_KEY)
          {            
            if (eUserPlayer.u8HeartPoints == 0 || eCpuPlayer.u8HeartPoints == 0)
            {
              eCurrentState = eEndGame; 
            }
            else
            {
              // Ninguém perdeu, volta para selecionar novos ataques
              eCurrentState = eBattleInit;
              u8ContAttack = 0; // Prepara para o novo round
              selectedOption = 0;
              memset((void*)eUserPlayer.eAttackSequential, 0, sizeof(eUserPlayer.eAttackSequential));
              
              // Gera novos ataques para a CPU para o próximo round
              for(uint8_t u8Idx = 0; u8Idx < ATTACKS_NUMBERS; u8Idx++)
              {
                eCpuPlayer.eAttackSequential[u8Idx] = (EColor)(rand() % 6);
              }
            }
            u8CleanScreen = TRUE;
          }
          break;
        }
        case eEndGame:
        {
          if(cLocalKeyPressed == CONFIRM_KEY)
          {
            eCurrentState = eInitGame;
            u8CleanScreen = TRUE;
          }
          break;
        }
        default:
        {
          // Estado desconhecido, volta para o início por segurança
          eCurrentState = eInitGame;
          u8CleanScreen = TRUE;
          break;
        }
      }
      osMutexRelease(gameMutexHandle);
    }
    osDelay(50);
  }
}

void StartDisplayTask(void const * argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  char buffer[30];
  uint8_t u8RedrawScreen = FALSE;
  EGameStates ePreviousState = eInitGame;
  for(;;)
  {
    osMutexWait(gameMutexHandle, osWaitForever);
    EGameStates eLocalCurrentState = eCurrentState;
    osMutexRelease(gameMutexHandle);



    if(eLocalCurrentState != ePreviousState)
    {
      ClearScreen();

      switch(eLocalCurrentState)
      {
          case eInitGame:
          {
            ILI9488_DrawImage_RGB666(100, 10, Logo300_map.width, Logo300_map.height, Logo300_map.pixel_data);
            break;
          }
          case eDificultSelect:
          {
            ILI9488_DrawImage_RGB666(0, 0, background_map.width, background_map.height, background_map.pixel_data);
            
            DrawMenu("Selecione Dificuldade", difficultyOptions, MENU_OPTIONS_DIFFICULTY, selectedOption);
            break;
          }
          case ePersonaSelect:
          {
            DrawMenu("Selecione Personagem", personaOptions, MENU_OPTIONS_PERSONA, selectedOption);
            break;
          }
          case eBattleInit:
          {
            ILI9488_WriteString(10, 15, "Prepare seus ataques!", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
            ILI9488_WriteString(10, 35, "Posicione os 4 cubos e pressione *", Font_7x10, ILI9488_YELLOW, ILI9488_BLACK);
            for (int i = 0; i < 4; i++) {
              sprintf(buffer, "Sensor %d:", i + 1);
              ILI9488_WriteString(20, 80 + (i * 30), buffer, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
              }
            break;
          }
          case ePlayerTurn:
          {
            ILI9488_WriteString(10, 20, "Resultado do Round", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
            sprintf(buffer, "Sua Vida: %d", eUserPlayer.u8HeartPoints);
            ILI9488_WriteString(10, 60, buffer, Font_7x10, ILI9488_GREEN, ILI9488_BLACK);
            sprintf(buffer, "Vida CPU: %d", eCpuPlayer.u8HeartPoints);
            ILI9488_WriteString(10, 90, buffer, Font_7x10, ILI9488_RED, ILI9488_BLACK);
            ILI9488_WriteString(10, 130, "Seus Ataques:", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
            for(uint8_t i = 0; i < ATTACKS_NUMBERS; i++) {
                uint16_t attackColor = ILI9488_WHITE;
                switch(eUserPlayer.eAttackSequential[i]) {
                    case eRed:    attackColor = ILI9488_RED;   break;
                    case eBlue:   attackColor = ILI9488_BLUE;  break;
                    case eGreen:  attackColor = ILI9488_CYAN;  break;
                    case eYellow: attackColor = ILI9488_BROWN; break;
                    case eWhite:  attackColor = ILI9488_WHITE; break;
                    case eBlack:  attackColor = ILI9488_GRAY;  break;
                }
                ILI9488_FillRectangle(10 + (i * 30), 150, 20, 20, attackColor);
            }
            ILI9488_WriteString(10, 190, "Ataques CPU:", Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
            for(uint8_t i = 0; i < ATTACKS_NUMBERS; i++) {
                uint16_t attackColor = ILI9488_WHITE;
                switch(eCpuPlayer.eAttackSequential[i]) {
                    case eRed:    attackColor = ILI9488_RED;   break;
                    case eBlue:   attackColor = ILI9488_BLUE;  break;
                    case eGreen:  attackColor = ILI9488_CYAN;  break;
                    case eYellow: attackColor = ILI9488_BROWN; break;
                    case eWhite:  attackColor = ILI9488_WHITE; break;
                    case eBlack:  attackColor = ILI9488_GRAY;  break;
                }
                ILI9488_FillRectangle(10 + (i * 30), 210, 20, 20, attackColor);
            }
            ILI9488_WriteString(10, 280, "Pressione * para continuar...", Font_7x10, ILI9488_YELLOW, ILI9488_BLACK);
            break;
          }
          case eEndGame:
          {
            if (eUserPlayer.u8HeartPoints > 0) {
                ILI9488_WriteString(70, 80, "VITORIA!", Font_7x10, ILI9488_GREEN, ILI9488_BLACK);
            } else {
                ILI9488_WriteString(70, 80, "DERROTA!", Font_7x10, ILI9488_RED, ILI9488_BLACK);
            }
            sprintf(buffer, "Sua Vida Final: %d", eUserPlayer.u8HeartPoints);
            ILI9488_WriteString(10, 140, buffer, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
            sprintf(buffer, "Vida Final CPU: %d", eCpuPlayer.u8HeartPoints);
            ILI9488_WriteString(10, 160, buffer, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
            ILI9488_WriteString(10, 250, "Pressione * para recomecar", Font_7x10, ILI9488_YELLOW, ILI9488_BLACK);
            break;
          }
          default:
          {
            ILI9488_WriteString(10, 10, "Erro de Estado!", Font_7x10, ILI9488_RED, ILI9488_BLACK);
            break;
          }
      }
      ePreviousState = eLocalCurrentState;    
    }

    if (eLocalCurrentState == eBattleInit)
    {
        for (int i = 0; i < 4; i++) {
            EColor detectedColor = TCS3472_DetectColor(colorData[i]);
            uint16_t colorBox = ILI9488_BLACK;
            char colorName[10] = "Vazio";

            switch(detectedColor) {
                case eRed:    colorBox = ILI9488_RED;   strcpy(colorName, "Fogo");   break;
                case eBlue:   colorBox = ILI9488_BLUE;  strcpy(colorName, "Agua");   break;
                case eGreen:  colorBox = ILI9488_CYAN;  strcpy(colorName, "Ar");     break;
                case eYellow: colorBox = ILI9488_BROWN; strcpy(colorName, "Terra");  break;
                case eWhite:  colorBox = ILI9488_WHITE; strcpy(colorName, "Luz?");   break;
                default: break;
            }

            // APAGA a área do nome da cor antiga desenhando um retângulo preto por cima
            ILI9488_FillRectangle(100, 80 + (i * 30), 45, 10, ILI9488_BLACK);
            // Escreve o novo nome da cor
            ILI9488_WriteString(100, 80 + (i * 30), colorName, Font_7x10, ILI9488_WHITE, ILI9488_BLACK);
            // Redesenha o quadrado colorido
            ILI9488_FillRectangle(150, 75 + (i * 30), 20, 20, colorBox);
        }
    }

    HAL_Delay(10);
  }
  /* USER CODE END StartDisplayTask */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
