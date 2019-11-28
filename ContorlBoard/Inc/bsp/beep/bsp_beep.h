#ifndef __BSP_BEEP_H__
#define __BSP_BEEP_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* 类型定义 ------------------------------------------------------------------*/
typedef enum
{
  BEEPState_OFF = 0,
  BEEPState_ON,
}BEEPState_TypeDef;
#define IS_BEEP_STATE(STATE)           (((STATE) == BEEPState_OFF) || ((STATE) == BEEPState_ON))

/* 宏定义 --------------------------------------------------------------------*/
#define BEEP_RCC_CLK_ENABLE()         __HAL_RCC_GPIOD_CLK_ENABLE()
#define BEEP_GPIO_PIN                 GPIO_PIN_7
#define BEEP_GPIO                     GPIOD

#define BEEP_ON                       HAL_GPIO_WritePin(BEEP_GPIO,BEEP_GPIO_PIN,GPIO_PIN_SET)    // 输出高电平
#define BEEP_OFF                      HAL_GPIO_WritePin(BEEP_GPIO,BEEP_GPIO_PIN,GPIO_PIN_RESET)  // 输出低电平
#define BEEP_TOGGLE                   HAL_GPIO_TogglePin(BEEP_GPIO,BEEP_GPIO_PIN)                // 输出反转


/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/
void BEEP_GPIO_Init(void);
void BEEP_StateSet(BEEPState_TypeDef state);
  
#endif  // __BSP_BEEP_H__

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
