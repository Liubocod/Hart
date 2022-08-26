/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_hal.h"
#include "adc.h"
#include "crc.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "FRAM.h"
#include "gpio.h"
#include "initSystem.h"
#include "Eeprom.h"
#include "ComLCDHandle.h"
#include "EchoHandle.h"
#include "DistanceDataHandle.h"
#include "HartHandle.h"
#include "RFHandle.h"
#include "IWR1443.h"
#include "HandleADSample.h"
#include "perf_management.h"
#include "LCD485.h"
#include "BlueTooth_Com.h"
#include "CopyRightActivate.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
//uint8_t RxTestdata[20];
uint16_t UpdateDistanceCount;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

__no_init uint16_t PowerUpFlag  @0x10000000;

int main(void)
{
  /* MCU Configuration----------------------------------------------------------*/
	uint16_t TempSecond;
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	SystemClock_Config();
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM8_Init();
//	CurrentFloat(REBOOTCURRENT);
	InitIWR1443Para();
	MX_CRC_Init();
	DISABLE_LCDVCC;
	MX_DMA_Init();
	MX_I2C2_Init();
//	MX_I2C4_Init();
	MX_SPI2_Init();
	MX_UART4_Init();
	MX_TIM4_Init();
	MX_USART1_UART_Init();
	do
	{
	}
	while(ReadLowVcon_Status() == GPIO_PIN_RESET);
	InitPara();
	InitFRAM();
	HART_RESET_RESET;
	InitPort();
	ReadPerfFormFRAM();
//	HAL_SuspendTick();
	UpdateDistanceCount = 0;
	HART_RESET_SET;
	InitADSamplePara();
//	InitUartPara();
	EnableBTReceive();
	MX_IWDG_Init();
	ENABLE_VCC_BLTOOTH;
	Set_BLTooth_Adv_Data(1);
	ENABLE_RESET_BLTOOTH;
	DIR_485B_RX;
//	DISABLE_RESET_BLTOOTH;
	G_EnableActivatorAskState = TRUE;   // G_EnableActivatorAskState=TRUE时，执行 CheckActivationCode()函数会
	if(PowerUpFlag == 0x5AA5)
	{
		RecordRebootTimeToFRAM(SOFTREBOOTCOUNT_INDEX);
	}
	else
	{
		PowerUpFlag = 0x5AA5;
		RecordRebootTimeToFRAM(HARDREBOOTCOUNT_INDEX);
	}
	UpdateDistanceCount = GetRTCSecond();
	SetReBoot_Current();
	MX_RTC_Init();
	MX_I2C1_Init();
	InitLcdData();
        EnableHartRx();
	while (1)
	{
//		ReadRTCTime();
		Feed_IWDG();
		if(ReadLowVcon_Status() == GPIO_PIN_SET)
		{
			LcdHandle();
			Set_BLT_Adv_Data();/*发送蓝牙广播包数据*/
			AnalyzeHartProtocol();
			ModbusProtocol();
                        
			BT_ModbusProtocol();
			HandleIWR1443_ControlFlow();
			TempSecond = GetRTCSecond();
			if(Get_IWR1443_Status() == IWR1443_WAIT_STATUS)
			{
				if(UpdateDistanceCount != TempSecond)
				{
					UpdateDistanceCount = TempSecond;
					DistanceEchoProccess();
					SaveModifyParaInfo();
					RecordMeasureValureMax_Min();
					IncCheckActivateCodeTimer();
					RecordMeterWorkTime();
					CheckRamPara();
					VailEchoConfirm_IncTime();
//					UpdateLCDCurveData();
					CreatFakeWavePointIndex();
//					RecordBTInf();
				}
				SaveUserPara();/*分时保存用户设定的参数*/
			}
			HandleADSample();
		}
		else
		{
			AbnormalCloseWR1443Vcc();
		}
		HAL_PWREx_EnableLowPowerRunMode();
		__WFI();
	}
}

/** System Clock Configuration*/
void	SystemClock_Config(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
//	RCC_OscInitStruct.HSICalibrationValue = 16;
//  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
//  RCC_OscInitStruct.MSICalibrationValue = 0;
//  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_8;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

    /**Initializes the CPU, AHB and APB busses clocks 
    */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;//RCC_SYSCLKSOURCE_MSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
		|RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_I2C3|RCC_PERIPHCLK_ADC;
	PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
	PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;
	PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_SYSCLK;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
	PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_SYSCLK;
	PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_SYSCLK;
	PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;			//时钟的晶振选择
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the main internal regulator output voltage */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);

	/**Configure the Systick interrupt time */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
