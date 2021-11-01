/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define Puls_DX_Pin GPIO_PIN_3
#define Puls_DX_GPIO_Port GPIOE
#define Puls_DX_EXTI_IRQn EXTI3_IRQn
#define Puls_OK_Pin GPIO_PIN_5
#define Puls_OK_GPIO_Port GPIOE
#define Puls_OK_EXTI_IRQn EXTI9_5_IRQn
#define Puls_SX_Pin GPIO_PIN_13
#define Puls_SX_GPIO_Port GPIOC
#define Puls_SX_EXTI_IRQn EXTI15_10_IRQn
#define Reset_DISP_Pin GPIO_PIN_6
#define Reset_DISP_GPIO_Port GPIOC
#define Buzzer_Pin GPIO_PIN_8
#define Buzzer_GPIO_Port GPIOC
#define D4_Pin GPIO_PIN_0
#define D4_GPIO_Port GPIOD
#define D5_Pin GPIO_PIN_2
#define D5_GPIO_Port GPIOD
#define E_Pin GPIO_PIN_3
#define E_GPIO_Port GPIOD
#define D6_Pin GPIO_PIN_4
#define D6_GPIO_Port GPIOD
#define RS_Pin GPIO_PIN_5
#define RS_GPIO_Port GPIOD
#define D7_Pin GPIO_PIN_6
#define D7_GPIO_Port GPIOD
#define Puls_MENU_Pin GPIO_PIN_1
#define Puls_MENU_GPIO_Port GPIOE
#define Puls_MENU_EXTI_IRQn EXTI1_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
