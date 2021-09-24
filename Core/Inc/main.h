/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GD32_X_CS_Pin GPIO_PIN_2
#define GD32_X_CS_GPIO_Port GPIOE
#define GD32_SIG_SD_DETECT_Pin GPIO_PIN_8
#define GD32_SIG_SD_DETECT_GPIO_Port GPIOB
#define GD32_LCD_BACKLIGHT_Pin GPIO_PIN_5
#define GD32_LCD_BACKLIGHT_GPIO_Port GPIOB
#define GD32_X2_EN_Pin GPIO_PIN_14
#define GD32_X2_EN_GPIO_Port GPIOG
#define GD32_Z_STP_Pin GPIO_PIN_13
#define GD32_Z_STP_GPIO_Port GPIOG
#define GD32_B_DIR_Pin GPIO_PIN_4
#define GD32_B_DIR_GPIO_Port GPIOB
#define GD32_FAN_NOZ_E_Pin GPIO_PIN_3
#define GD32_FAN_NOZ_E_GPIO_Port GPIOB
#define GD32_Y_EN_Pin GPIO_PIN_7
#define GD32_Y_EN_GPIO_Port GPIOD
#define GD32_FAN_NOZ_B_Pin GPIO_PIN_15
#define GD32_FAN_NOZ_B_GPIO_Port GPIOA
#define GD32_SIG_DOOR_Pin GPIO_PIN_4
#define GD32_SIG_DOOR_GPIO_Port GPIOE
#define GD32_HEAT_BED_Pin GPIO_PIN_5
#define GD32_HEAT_BED_GPIO_Port GPIOE
#define GD32_Z_DIR_Pin GPIO_PIN_6
#define GD32_Z_DIR_GPIO_Port GPIOE
#define GD32_BUZZER_Pin GPIO_PIN_9
#define GD32_BUZZER_GPIO_Port GPIOB
#define GD32_X2_STP_Pin GPIO_PIN_7
#define GD32_X2_STP_GPIO_Port GPIOB
#define GD32_X2_DIR_Pin GPIO_PIN_6
#define GD32_X2_DIR_GPIO_Port GPIOB
#define GD32_E_EN_Pin GPIO_PIN_12
#define GD32_E_EN_GPIO_Port GPIOG
#define GD32_B_EN_Pin GPIO_PIN_10
#define GD32_B_EN_GPIO_Port GPIOG
#define GD32_E_STP_Pin GPIO_PIN_6
#define GD32_E_STP_GPIO_Port GPIOD
#define GD32_TOUCH_MOSI_Pin GPIO_PIN_3
#define GD32_TOUCH_MOSI_GPIO_Port GPIOI
#define GD32_LIGHT_BAR_Pin GPIO_PIN_13
#define GD32_LIGHT_BAR_GPIO_Port GPIOC
#define GD32_TOUCH_CS_Pin GPIO_PIN_8
#define GD32_TOUCH_CS_GPIO_Port GPIOI
#define GD32_B_STP_Pin GPIO_PIN_3
#define GD32_B_STP_GPIO_Port GPIOD
#define GD32_SIG_Z_MAX_Pin GPIO_PIN_14
#define GD32_SIG_Z_MAX_GPIO_Port GPIOC
#define GD32_X2_CS_Pin GPIO_PIN_11
#define GD32_X2_CS_GPIO_Port GPIOI
#define GD32_SIG_Z_MIN_Pin GPIO_PIN_15
#define GD32_SIG_Z_MIN_GPIO_Port GPIOC
#define GD32_SIG_X_Pin GPIO_PIN_2
#define GD32_SIG_X_GPIO_Port GPIOH
#define GD32_FAN_EB_MOTOR_Pin GPIO_PIN_8
#define GD32_FAN_EB_MOTOR_GPIO_Port GPIOA
#define GD32_X_STP_Pin GPIO_PIN_3
#define GD32_X_STP_GPIO_Port GPIOH
#define GD32_Z_EN_Pin GPIO_PIN_7
#define GD32_Z_EN_GPIO_Port GPIOC
#define GD32_X_DIR_Pin GPIO_PIN_4
#define GD32_X_DIR_GPIO_Port GPIOH
#define GD32_SIG_MAT_E1_Pin GPIO_PIN_6
#define GD32_SIG_MAT_E1_GPIO_Port GPIOC
#define GD32_X_EN_Pin GPIO_PIN_5
#define GD32_X_EN_GPIO_Port GPIOH
#define GD32_STEERING_ENGINE_Pin GPIO_PIN_6
#define GD32_STEERING_ENGINE_GPIO_Port GPIOF
#define GD32_TOUCH_MISO_Pin GPIO_PIN_3
#define GD32_TOUCH_MISO_GPIO_Port GPIOG
#define GD32_Y_CS_Pin GPIO_PIN_1
#define GD32_Y_CS_GPIO_Port GPIOC
#define GD32_LIGHT_Pin GPIO_PIN_2
#define GD32_LIGHT_GPIO_Port GPIOB
#define GD32_TOUCH_SCK_Pin GPIO_PIN_6
#define GD32_TOUCH_SCK_GPIO_Port GPIOH
#define GD32_E_DIR_Pin GPIO_PIN_8
#define GD32_E_DIR_GPIO_Port GPIOH
#define GD32_FAN_FILTER_Pin GPIO_PIN_13
#define GD32_FAN_FILTER_GPIO_Port GPIOD
#define GD32_HEAT_NOZ_E_Pin GPIO_PIN_1
#define GD32_HEAT_NOZ_E_GPIO_Port GPIOA
#define GD32_TEMP_B_Pin GPIO_PIN_0
#define GD32_TEMP_B_GPIO_Port GPIOA
#define GD32_Z_CS_Pin GPIO_PIN_4
#define GD32_Z_CS_GPIO_Port GPIOA
#define GD32_E_CS_Pin GPIO_PIN_4
#define GD32_E_CS_GPIO_Port GPIOC
#define GD32_TOUCH_PEN_Pin GPIO_PIN_7
#define GD32_TOUCH_PEN_GPIO_Port GPIOH
#define GD32_HEAT_NOZ_B_Pin GPIO_PIN_2
#define GD32_HEAT_NOZ_B_GPIO_Port GPIOA
#define GD32_TEMP_BED_Pin GPIO_PIN_6
#define GD32_TEMP_BED_GPIO_Port GPIOA
#define GD32_SIG_MAT_E0_Pin GPIO_PIN_5
#define GD32_SIG_MAT_E0_GPIO_Port GPIOA
#define GD32_B_CS_Pin GPIO_PIN_5
#define GD32_B_CS_GPIO_Port GPIOC
#define GD32_Y_STP_Pin GPIO_PIN_12
#define GD32_Y_STP_GPIO_Port GPIOB
#define GD32_SIG_Y_Pin GPIO_PIN_13
#define GD32_SIG_Y_GPIO_Port GPIOB
#define GD32_TEMP_E_Pin GPIO_PIN_3
#define GD32_TEMP_E_GPIO_Port GPIOA
#define GD32_SIG_X2_Pin GPIO_PIN_7
#define GD32_SIG_X2_GPIO_Port GPIOA
#define GD32_TEMP_CAVITY_Pin GPIO_PIN_1
#define GD32_TEMP_CAVITY_GPIO_Port GPIOB
#define GD32_HEAT_CAVITY_Pin GPIO_PIN_0
#define GD32_HEAT_CAVITY_GPIO_Port GPIOB
#define GD32_Y_DIR_Pin GPIO_PIN_10
#define GD32_Y_DIR_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
