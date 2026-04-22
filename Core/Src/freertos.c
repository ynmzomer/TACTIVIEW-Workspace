/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_hal.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief  FreeRTOS idle hook — called by the idle task when no other task is ready.
 *
 * __WFI (Wait For Interrupt) puts the Cortex-M4 into SLEEP mode:
 *   - CPU pipeline halts → ~0 dynamic power
 *   - All peripheral clocks (USART1, USART3, TIM6, I2C1) stay active
 *   - Any enabled interrupt (UART RX, SysTick, PendSV, …) wakes the CPU
 *
 * When configUSE_TICKLESS_IDLE = 1, vPortSuppressTicksAndSleep() handles
 * multi-tick idle periods automatically (stops SysTick, issues __WFI,
 * restores tick count via vTaskStepTick). This hook covers the short
 * single-tick gaps that tickless does not suppress.
 *
 * TIM6 (HAL_GetTick timebase) is NOT touched — sensor timestamps remain accurate.
 */
void vApplicationIdleHook(void)
{
    __WFI();
}

/* USER CODE END Application */

