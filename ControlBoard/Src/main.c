/**
  ******************************************************************************
  * 文件名程: main.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2015-10-04
  * 功    能: WiFi(ESP8266)底层驱动实现
  ******************************************************************************
  * 说明：
  * 本例程配套硬石stm32开发板YS-F1Pro使用。
  * 
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */
/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "usart/bsp_debug_usart.h"
#include "ESP8266/bsp_esp8266.h"
#include "beep/bsp_beep.h"
#include "led/bsp_led.h"
#include "stdlib.h"
#include "tim.h"
#include <stdlib.h>
/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
#define User_ESP8266_ApSsid                       "HUAWEI P20"              //要连接的热点的名称
#define User_ESP8266_ApPwd                        "12345678"           //要连接的热点的密钥

#define User_ESP8266_TcpServer_IP                 "10.203.8.178"       //要连接的服务器的 IP
#define User_ESP8266_TcpServer_Port               "8080"                 //要连接的服务器的端口

/* 私有变量 ------------------------------------------------------------------*/
/* 扩展变量 ------------------------------------------------------------------*/
extern __IO uint8_t ucTcpClosedFlag;

uint8_t aRx;
uint8_t aRxbuffer[255];
uint8_t aRxCount =0 ;

int TIME=0;

int StartFlag =0;
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: 系统时钟配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;  // 外部晶振，8MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  // 9倍频，得到72MHz主时钟
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;       // 系统时钟：72MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;              // AHB时钟：72MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;               // APB1时钟：36MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;               // APB2时钟：72MHz
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

 	// HAL_RCC_GetHCLKFreq()/1000    1ms中断一次
	// HAL_RCC_GetHCLKFreq()/100000	 10us中断一次
	// HAL_RCC_GetHCLKFreq()/1000000 1us中断一次
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);  // 配置并启动系统滴答定时器
  /* 系统滴答定时器时钟源 */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  /* 系统滴答定时器中断优先级配置 */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * 函数功能: 主函数.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
int main(void)
{  
  uint8_t ucStatus;  
  uint8_t pCH;
  
  /* 复位所有外设，初始化Flash接口和系统滴答定时器 */
  HAL_Init();
  /* 配置系统时钟 */
  SystemClock_Config();

  LED_GPIO_Init();
  BEEP_GPIO_Init();
		LED1_ON;LED2_ON;LED3_ON;LED4_ON;
  
  /* 初始化串口并配置串口中断优先级 */
  MX_DEBUG_USART_Init();
  MX_TIM2_Init();
	MX_USART3_UART_Init();
  ESP8266_Init();
  
  ESP8266_Init();
  printf("正在配置 ESP8266 ......\n" );
  
  if(ESP8266_AT_Test())
  {
    printf("AT test OK\n");
  }
  printf("\n< 1 >\n");
	if(ESP8266_Net_Mode_Choose(STA))
  {
    printf("ESP8266_Net_Mode_Choose OK\n");
  }  
  printf("\n< 2 >\n");
  while(!ESP8266_JoinAP(User_ESP8266_ApSsid,User_ESP8266_ApPwd));		
	printf("\n< 3 >\n");
  ESP8266_Enable_MultipleId(DISABLE);	
	while(!ESP8266_Link_Server(enumTCP,User_ESP8266_TcpServer_IP,User_ESP8266_TcpServer_Port,Single_ID_0));	
	printf("\n< 4 >\n");
  while(!ESP8266_UnvarnishSend());	
	printf("配置 ESP8266 完毕\n");
	HAL_UART_Receive_IT(&huart3,(uint8_t *)&aRx, 1);
  __HAL_UART_ENABLE_IT(&husartx_esp8266,UART_IT_IDLE);
	HAL_TIM_Base_Start_IT(&htim2);

  
  /* 无限循环 */
  while (1)
  {
    ESP8266_ReceiveString(ENABLE);
    if(strEsp8266_Fram_Record .InfBit .FramFinishFlag )
		{
    	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';
      printf ( "\r\n%s\r\n", strEsp8266_Fram_Record .Data_RX_BUF );
      /*将接收到的字符串转成整形数*/
      pCH=atoi(strEsp8266_Fram_Record .Data_RX_BUF);
       switch(pCH)
       {
         case 0:
				 LED1_OFF;
				 StartFlag=1;
         break;
         case 1:
				 LED1_ON;
				 StartFlag =0;
         break;
         case 2:
         LED2_OFF;
				 StartFlag=1;
         break;
         case 3:
				 LED2_ON;
				 StartFlag =0;
         break;
         case 4:
         LED3_OFF;
				 StartFlag=1;
         break;
         case 5:
				 LED3_ON;
				 StartFlag =0;
         break;
         case 6:
         LED4_OFF;
				 StartFlag=1;
         break;
         case 7:
				 LED4_ON;
				 StartFlag =0;
         break;
       }         
    }
    if(ucTcpClosedFlag)                                             //检测是否失去连接
		{
			ESP8266_ExitUnvarnishSend();                                    //退出透传模式			
			do ucStatus = ESP8266_Get_LinkStatus();                         //获取连接状态
			while(!ucStatus);			
			if(ucStatus==4)                                             //确认失去连接后重连
			{
				printf("正在重连热点和服务器 ......\n");				
				while(!ESP8266_JoinAP(User_ESP8266_ApSsid,User_ESP8266_ApPwd));				
				while(!ESP8266_Link_Server(enumTCP,User_ESP8266_TcpServer_IP,User_ESP8266_TcpServer_Port,Single_ID_0));				
				printf("重连热点和服务器成功!!!\n");
			}			
			while(!ESP8266_UnvarnishSend());					
		}
  }
}

/**
  * 函数功能: 串口接收完成回调函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if(UartHandle->Instance==ESP8266_USARTx)
	{
  if(strEsp8266_Fram_Record.InfBit.FramLength<(RX_BUF_MAX_LEN-1))                       //预留1个字节写结束符
    strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++]=esp8266_rxdata;
  HAL_UART_Receive_IT(&husartx_esp8266,&esp8266_rxdata,1);
	}
	if(UartHandle->Instance==USART3)
	{
		aRxbuffer[aRxCount++]=aRx;

    if((aRxbuffer[aRxCount-1] == 0x0A)&&(aRxbuffer[aRxCount-2] == 0x0D)) //判断结束位
    {
        aRxCount = 0;
        memset(aRxbuffer,0x00,sizeof(aRxbuffer)); //清空数组
    }
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&aRx, 1); 
		if(aRxbuffer[0]=='1')
		{
			LED1_OFF;
		}
		if(aRxbuffer[0]=='2')
		{
			LED2_OFF;
		}
		if(aRxbuffer[0]=='3')
		{
			LED3_OFF;
		}
		if(aRxbuffer[0]=='4')
		{
			LED4_OFF;
		}
		StartFlag =1;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
			if(StartFlag)
			{
			ESP8266_Usart ( "停车时间为%ds\r\n", TIME++);
				if(TIME<=20)
				{
				ESP8266_Usart ( "停车费用为%d元\r\n", 3);
				}
				if(TIME>20)
				{
				ESP8266_Usart ( "停车费用为%d元\r\n", (int)((((TIME-20)/10)*0.5)+3));
				}
			}
    }
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
