/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "lwip.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include "lwip/apps/httpd.h"
#include "LCD.h"

#define NUM_SENSORS (5)
#define CRITICAL_TEMP_DEFAULT (27)
#define CRITICAL_TEMP_MAX (40)
#define CRITICAL_TEMP_MIN (25)
#define TIMES_ARRAY_DIM (25)
#define NUM_SENSORS_MAX (NUM_SENSORS-1)
#define TIMES_ARRAY_DIM_MAX (TIMES_ARRAY_DIM-1)
#define UART_REFRESH_TIME_DEFAULT (5)

#define SX      (4)
#define DX      (8)
#define OK      (16)
#define MENU    (1)
#define IMP     (2)
#define ALARM   (32)
#define CRIT    (64)
#define MENUSX  (SX+MENU)
#define MENUDX  (DX+MENU)
#define MENUOK  (OK+MENU)
#define IMPSX   (SX+IMP)
#define IMPDX   (DX+IMP)
#define IMPOK   (OK+IMP)
#define CRITSX  (SX+CRIT)
#define CRITDX  (DX+CRIT)
#define CRITOK  (OK+CRIT)
#define ALARMOK (OK+ALARM)


#define MAINPAGE (0)
#define MENUPAGE (1)
#define UARTPAGE (2)
#define CRITPAGE (3)

#define GRADO (0xDF)
#define FRECCIA_DESTRA (0x7E)

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
volatile uint8_t UARTready=0;
volatile uint8_t halfSec;
volatile uint8_t premuto;
volatile uint8_t tasto;
volatile uint32_t ADCVal[NUM_SENSORS];
uint32_t ADCValBuffer[NUM_SENSORS];
float temp[NUM_SENSORS];
const  char* tagSSI[NUM_SENSORS] = { "t1", "t2", "t3", "t4", "t5" };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_UART4_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
static void MX_NVIC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

uint16_t handlerSSI(int iIndex, char *pcInsert, int iInsertLen){
    switch(iIndex){
      case 0:
        sprintf(pcInsert,"%.0f",temp[0]);
        return strlen(pcInsert);
      case 1:
        sprintf(pcInsert,"%.0f",temp[1]);
        return strlen(pcInsert);
      case 2:
        sprintf(pcInsert,"%.0f",temp[2]);
        return strlen(pcInsert);
      case 3:
        sprintf(pcInsert,"%.0f",temp[3]);
        return strlen(pcInsert);
      case 4:
        sprintf(pcInsert,"%.0f",temp[4]);
        return strlen(pcInsert);
      default:
        return 0;
    }
}

void initSSI(void){
    http_set_ssi_handler(handlerSSI, (const char**) tagSSI, NUM_SENSORS);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  // COSTANTI

  const uint8_t times[TIMES_ARRAY_DIM] = {1,2,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,150,200,250};

  // VARIABILI

  char stringTemp[9]="00001000\0";
  uint8_t selImpostazioni=UARTPAGE;
  uint8_t sel_index=0;
  uint8_t zoneNumber=0;
  uint8_t secElapsed=0;
  uint8_t secElapsedOld=0;
  uint8_t halfSecOld=0;
  uint8_t isTimeToSendToUART=0;
  uint8_t UARTtime=UART_REFRESH_TIME_DEFAULT;
  uint8_t pagina=0;
  uint8_t numPagina=0;
  uint8_t inMenu=0;
  uint8_t inImp=0;
  uint8_t inAlarm=0;
  uint8_t inCrit=0;
  uint8_t silenzia=0;
  uint8_t sendRefreshTime=UART_REFRESH_TIME_DEFAULT;
  uint8_t times_index=2;
  uint8_t new_times_index=2;
  uint8_t criticalTemp=CRITICAL_TEMP_DEFAULT;
  uint8_t critTempBuf=CRITICAL_TEMP_DEFAULT;

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_DMA_Init();
  MX_UART4_Init();
  MX_TIM2_Init();
  MX_LWIP_Init();
  MX_ADC1_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start_IT(&htim2);
  HAL_ADC_Start_DMA(&hadc1,ADCValBuffer,NUM_SENSORS);
  httpd_init();
  initSSI();

  // Resetto il display a 7 segmenti
  HAL_GPIO_WritePin(GPIOC, Reset_DISP_Pin, GPIO_PIN_SET);
  HAL_Delay(1000);
  HAL_GPIO_WritePin(GPIOC, Reset_DISP_Pin, GPIO_PIN_RESET);

  lcd.GPIOx=GPIOD;
  lcd.D4 = 0;
  lcd.D5 = 2;
  lcd.D6 = 4;
  lcd.D7 = 6;
  lcd.E  = 3;
  lcd.RS = 5;

  LCD_Init(DUAL_ROW,NORMAL_FONT,TEXT_LEFT_TO_RIGHT,TEXT_SHIFT_NO);
  LCD_curMode(CURSOR_OFF,BLINK_OFF);
  LCD_curSetHome();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

      MX_LWIP_Process();

      secElapsed = halfSec&1 ? secElapsed : halfSec>>1;

      if(secElapsedOld!=secElapsed){

          secElapsedOld=secElapsed;
          isTimeToSendToUART++;

          if(isTimeToSendToUART==UARTtime){

                // INVIO TRAMITE UART

                isTimeToSendToUART=0;

                sprintf(stringTemp,"00%03d%03d",zoneNumber+1,(uint8_t)temp[zoneNumber]);
                uint8_t i=0;
                while(stringTemp[i]!=0){
                  HAL_UART_Transmit_IT(&huart4, (uint8_t*)&stringTemp[i], 1);
                  while(!UARTready);
                  UARTready=0;
                  HAL_Delay(2);
                  i++;
                }

                zoneNumber++;
                zoneNumber%=NUM_SENSORS;
          }

      }

      if(halfSec!=halfSecOld){
          halfSecOld=halfSec;

          for(int i=0; i<NUM_SENSORS; i++){
              temp[i]=ADCVal[i]/40.95;
          }

          sel_index=selImpostazioni-2;

          // GESTIONE PULSANTI

          if(premuto){
              premuto=0;
              switch(tasto | inMenu | inImp | inCrit | inAlarm){
                case (SX):
                  numPagina--;
                  if(numPagina>NUM_SENSORS) numPagina=NUM_SENSORS_MAX;
                  break;
                case (DX):
                  numPagina++;
                  numPagina%=(NUM_SENSORS);
                  break;
                case (MENU):
                  pagina^=MENUPAGE; //fa il toggle della pagina in modo che appena si preme menu si torna alla pagina iniziale
                  break;
                case (MENUSX):
                  selImpostazioni--;
                  if(selImpostazioni<UARTPAGE) selImpostazioni=CRITPAGE;
                  break;
                case (MENUDX):
                  selImpostazioni++;
                  if(selImpostazioni>CRITPAGE) selImpostazioni=UARTPAGE;
                  break;
                case (MENUOK):
                  pagina=selImpostazioni;
                  break;
                case (IMPSX):
                  times_index--;
                  if(times_index>TIMES_ARRAY_DIM) times_index=TIMES_ARRAY_DIM_MAX;
                  sendRefreshTime=times[times_index];
                  break;
                case (IMPDX):
                  times_index++;
                  times_index%=TIMES_ARRAY_DIM;
                  sendRefreshTime=times[times_index];
                  break;
                case (IMPOK):
                  pagina=MAINPAGE;
                  UARTtime=sendRefreshTime;
                  new_times_index=times_index;
                  isTimeToSendToUART=0;
                  break;
                case (CRITSX):
                  critTempBuf--;
                  if(critTempBuf<CRITICAL_TEMP_MIN) critTempBuf=CRITICAL_TEMP_MAX;
                  break;
                case (CRITDX):
                  critTempBuf++;
                  if(critTempBuf>CRITICAL_TEMP_MAX) critTempBuf=CRITICAL_TEMP_MIN;
                  break;
                case (CRITOK):
                  pagina=MAINPAGE;
                  criticalTemp=critTempBuf;
                  break;
                case (ALARMOK):
                  silenzia=1;
                  break;
                case (OK):
                default:
                  pagina=MAINPAGE;
                  break;
              }
          }

          // GESTIONE SCHERMATE

          switch(pagina){
            case MAINPAGE:
              inMenu=0;
              inImp=0;
              inCrit=0;
              selImpostazioni=UARTPAGE;
              sendRefreshTime=UARTtime;
              critTempBuf=criticalTemp;
              times_index=new_times_index;
              LCD_printf("Zona %d          Temp. %.0f%cC      ",(numPagina+1),temp[numPagina],GRADO);
              break;
            case MENUPAGE:
              inMenu=MENU;
              inImp=0;
              inCrit=0;
              LCD_curSetHome();
              LCD_print("Impostazioni");
              LCD_curSet(1,1);
              LCD_print("UART");
              LCD_curSet(1,7);
              LCD_print("T.critica");
              LCD_curSet(1,(!sel_index)*6);
              LCD_putch(' ');
              LCD_curSet(1,(sel_index)*6);
              LCD_putch(FRECCIA_DESTRA);
              break;
            case UARTPAGE:
              inMenu=0;
              inImp=IMP;
              inCrit=0;
              LCD_printf("UART: Aggiorna  ogni %d secondi   ",sendRefreshTime);
              break;
            case CRITPAGE:
              inMenu=0;
              inImp=0;
              inCrit=CRIT;
              LCD_printf("Temperatura     critica: %d%cC   ",critTempBuf,GRADO);
              break;
          }
      }

      if(temp[0]>=criticalTemp || temp[1]>=criticalTemp || temp[2]>=criticalTemp ||
         temp[3]>=criticalTemp || temp[4]>=criticalTemp){
             if(!silenzia){
                 inAlarm=ALARM;
                 HAL_GPIO_WritePin(GPIOC, Buzzer_Pin, GPIO_PIN_SET);
             }else{
                 inAlarm=0;
                 HAL_GPIO_WritePin(GPIOC, Buzzer_Pin, GPIO_PIN_RESET);
             }
      }else{
         inAlarm=0;
         silenzia=0;
         HAL_GPIO_WritePin(GPIOC, Buzzer_Pin, GPIO_PIN_RESET);
      }

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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* UART4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(UART4_IRQn);
  /* EXTI9_5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  /* EXTI1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* EXTI15_10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8399;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  huart4.Init.BaudRate = 19200;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Reset_DISP_Pin|Buzzer_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, D4_Pin|D5_Pin|E_Pin|D6_Pin 
                          |RS_Pin|D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Puls_DX_Pin Puls_OK_Pin Puls_MENU_Pin */
  GPIO_InitStruct.Pin = Puls_DX_Pin|Puls_OK_Pin|Puls_MENU_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : Puls_SX_Pin */
  GPIO_InitStruct.Pin = Puls_SX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Puls_SX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Reset_DISP_Pin Buzzer_Pin */
  GPIO_InitStruct.Pin = Reset_DISP_Pin|Buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : D4_Pin D5_Pin E_Pin D6_Pin 
                           RS_Pin D7_Pin */
  GPIO_InitStruct.Pin = D4_Pin|D5_Pin|E_Pin|D6_Pin 
                          |RS_Pin|D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
