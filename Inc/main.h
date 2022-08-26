/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/
#define	VERSION0				0x35
#define	VERSION1				0x30
#define	VERSION2				0x45 // 

#define	ADSAMPLECOUNTMAX					6
/* USER CODE BEGIN Includes */
#define	TSYSTEMCORECLOCK_APBH1					8000000
#define	TSYSTEMCORECLOCK_APBH2					16000000

#define	UPDATEDISTANCETIME					20/*50mS*20*/

#define	PERIODCENTER_26G					(TSYSTEMCORECLOCK_APBH2/28.5)
#define	PERIODCENTER_6G						(TSYSTEMCORECLOCK_APBH2/43.75)

#define	TIM1COUNTERCLOCK					16000000
#define	TIM1OUTCLOCK						200000

#define	TIM3COUNTERCLOCK					40000
#define	WDCLOCK							50

#define	TIM6COUNTERCLOCK					1000000
#define	TIM6OUTCLOCK						20

#define	TIM7COUNTERCLOCK					4000000
#define	TIM7OUTCLOCK						1000

#define	TIM8COUNTERCLOCK					8500000
#define	TIM8OUTCLOCK						130

#define	MODIFYPARAMODE_IIC					0
#define	MODIFYPARAMODE_LCD485					1
#define	MODIFYPARAMODE_BLUETL					2
#define	MODIFYPARAMODE_HART					3
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define TIM1_PRESCALERVALUE					(TSYSTEMCORECLOCK_APBH2/TIM1COUNTERCLOCK-1)
#define TIM1_PERIOD_VALUE					(TIM1COUNTERCLOCK/TIM1OUTCLOCK/2-1)
#define TIM1_PULSE_VALUE					TIM1_PERIOD_VALUE

#define TIM3_PRESCALERVALUE					(TSYSTEMCORECLOCK_APBH1/TIM3COUNTERCLOCK-1)
#define TIM3_PERIOD_VALUE					(TIM3COUNTERCLOCK/WDCLOCK/2-1)
#define TIM3_PULSE_VALUE					(TIM3_PERIOD_VALUE/2)

#define	TIM4COUNTERCLOCK					8000000
#define	TIM4OUTCLOCK						1100
#define TIM4_PRESCALERVALUE					(TSYSTEMCORECLOCK_APBH1/TIM4COUNTERCLOCK-1)
#define TIM4_PERIOD_VALUE					(TIM4COUNTERCLOCK/TIM4OUTCLOCK-1)
#define TIM4_PULSE_VALUE					(TIM4_PERIOD_VALUE/2)

#define TIM6_PRESCALERVALUE					(TSYSTEMCORECLOCK_APBH1/TIM7COUNTERCLOCK-1)
#define TIM6_PERIOD_VALUE					(TIM6COUNTERCLOCK/TIM6OUTCLOCK-1)

#define TIM7_PRESCALERVALUE					(TSYSTEMCORECLOCK_APBH1/TIM7COUNTERCLOCK-1)
#define TIM7_PERIOD_VALUE					(TIM7COUNTERCLOCK/TIM7OUTCLOCK-1)
#define	STARTRFVCCADDELAY					(TIM7COUNTERCLOCK/1000*13-1)						
#define	STARTONEMSDELAY						(TIM7COUNTERCLOCK/1000-1)						

#define TIM8_PRESCALERVALUE					(TSYSTEMCORECLOCK_APBH2/TIM8COUNTERCLOCK-1)
#define TIM8_PERIOD_VALUE					(TIM8COUNTERCLOCK/TIM8OUTCLOCK-1)
#define TIM8_PULSE_VALUE					(TIM8_PERIOD_VALUE/2)

#define RF_POWER_CON_Pin					GPIO_PIN_12
#define RF_POWER_CON_GPIO_Port					GPIOB
#define RF_MAXRAGNE_70_OR_35_Pin				GPIO_PIN_11
#define RF_MAXRAGNE_70_OR_35_GPIO_Port				GPIOD
#define RF_CLEAR_Pin						GPIO_PIN_10
#define RF_CLEAR_GPIO_Port					GPIOD
#define RF_FREQ_CON_S_Pin					GPIO_PIN_9
#define RF_FREQ_CON_S_GPIO_Port					GPIOD
#define RF_FREQ_CON_Pin						GPIO_PIN_8
#define RF_FREQ_CON_GPIO_Port					GPIOD

#define MEASURE_FREQU_PERIOD_Pin				GPIO_PIN_2
#define MEASURE_FREQU_PERIOD_GPIO_Port				GPIOA
#define RF_FREQU_INPUT_Pin					GPIO_PIN_3
#define RF_FREQU_INPUT_GPIO_Port				GPIOA
#define RF_FREQU_INPUT_EXTI_IRQn				EXTI3_IRQn
#define RF_FREQU_Pin						GPIO_PIN_11
#define RF_FREQU_GPIO_Port					GPIOE

#define VCONSTATUS_Pin						GPIO_PIN_3
#define VCONSTATUS_GPIO_Port					GPIOE

#define RF_26G_OR_6G_Pin					GPIO_PIN_3
#define RF_26G_OR_6G_GPIO_Port					GPIOE
#define MAXRANGE_70_OR_35_Pin					GPIO_PIN_4
#define MAXRANGE_70_OR_35_GPIO_Port				GPIOE

#define LCD_SCL_Pin						GPIO_PIN_8
#define LCD_SCL_GPIO_Port					GPIOB
#define LCD_SDA_Pin						GPIO_PIN_9
#define LCD_SDA_GPIO_Port					GPIOB

#define HART_RESET_Pin						GPIO_PIN_11
#define HART_RESET_GPIO_Port					GPIOA
#define LCDVCC_ON_OR_OFF_Pin					GPIO_PIN_0
#define LCDVCC_ON_OR_OFF_GPIO_Port				GPIOE
#define VCC5V_ON_OR_OFF_Pin					GPIO_PIN_7
#define VCC5V_ON_OR_OFF_GPIO_Port				GPIOC
#define VCCAD_CON_ON_OR_OFF_Pin					GPIO_PIN_10
#define VCCAD_CON_ON_OR_OFF_GPIO_Port				GPIOE

#define CURRENT_OUT_CON_Pin					GPIO_PIN_6
#define CURRENT_OUT_CON_GPIO_Port				GPIOC
#define WD_CLOCK_OUT_Pin					GPIO_PIN_4
#define WD_CLOCK_OUT_GPIO_Port					GPIOB

#define LSAMPLE_CLOCK_OUT_Pin					GPIO_PIN_9
#define LSAMPLE_CLOCK_OUT_GPIO_Port				GPIOB
#define IRW1443_RST_Pin						GPIO_PIN_9
#define IRW1443_RST_GPIO_Port					GPIOC

#define SPISCK_80G_Pin 						GPIO_PIN_13
#define SPISCK_80G_GPIO_Port					GPIOE
#define SPI_MISO_80G_Pin					GPIO_PIN_14
#define SPI_MISO_80G_GPIO_Port					GPIOE
#define SPI_MOSI_80G_Pin					GPIO_PIN_15
#define SPI_MOSI_80G_GPIO_Port					GPIOE

#define DIR_485B_Pin						GPIO_PIN_14
#define DIR_485B_GPIO_Port					GPIOE

#define EEROM_SCL_Pin						GPIO_PIN_13
#define EEROM_SCL_GPIO_Port					GPIOB
#define EEROM_SDA_Pin						GPIO_PIN_14
#define EEROM_SDA_GPIO_Port					GPIOB
#define EEROM_WP_Pin						GPIO_PIN_15
#define EEROM_WP_GPIO_Port					GPIOB

#define LCD_485_RX_Pin						GPIO_PIN_10
#define LCD_485_RX_GPIO_Port					GPIOB
#define LCD_485_TX_Pin						GPIO_PIN_11
#define LCD_485_TX_GPIO_Port					GPIOB
#define LCD_485_DE_Pin						GPIO_PIN_12
#define LCD_485_DE_GPIO_Port					GPIOB

#define 	BT_VCC_Pin					GPIO_PIN_3
#define		BT_VCC_GPIO_Port				GPIOC
/* USER CODE BEGIN Private defines */
#define	ENABLE_5V						HAL_GPIO_WritePin(VCC5V_ON_OR_OFF_GPIO_Port, VCC5V_ON_OR_OFF_Pin, GPIO_PIN_SET);
#define	DISABLE_5V						HAL_GPIO_WritePin(VCC5V_ON_OR_OFF_GPIO_Port, VCC5V_ON_OR_OFF_Pin, GPIO_PIN_RESET);

#define	ENABLE_LCDVCC						HAL_GPIO_WritePin(LCDVCC_ON_OR_OFF_GPIO_Port, LCDVCC_ON_OR_OFF_Pin, GPIO_PIN_RESET);
#define	DISABLE_LCDVCC						HAL_GPIO_WritePin(LCDVCC_ON_OR_OFF_GPIO_Port, LCDVCC_ON_OR_OFF_Pin, GPIO_PIN_SET);

#define	HART_RESET_SET						HAL_GPIO_WritePin(HART_RESET_GPIO_Port, HART_RESET_Pin, GPIO_PIN_SET);
#define	HART_RESET_RESET					HAL_GPIO_WritePin(HART_RESET_GPIO_Port, HART_RESET_Pin, GPIO_PIN_RESET);

#define	ENABLE_HART_RX						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
#define	ENABLE_HART_TX						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

#define	MAXRANGE						HAL_GPIO_ReadPin(MAXRANGE_70_OR_35_GPIO_Port,MAXRANGE_70_OR_35_Pin)
#define	RFFREQUENCY						HAL_GPIO_ReadPin(RF_26G_OR_6G_GPIO_Port,RF_26G_OR_6G_Pin)

#define	RF_MAXRANGE_35						HAL_GPIO_WritePin(RF_MAXRAGNE_70_OR_35_GPIO_Port, RF_MAXRAGNE_70_OR_35_Pin, GPIO_PIN_SET);
#define	RF_MAXRANGE_70						HAL_GPIO_WritePin(RF_MAXRAGNE_70_OR_35_GPIO_Port, RF_MAXRAGNE_70_OR_35_Pin, GPIO_PIN_RESET);
    
#define	RF_SIGNAL_SET						HAL_GPIO_WritePin(RF_CLEAR_GPIO_Port, RF_CLEAR_Pin, GPIO_PIN_SET);
#define	RF_SIGNAL_RESET						HAL_GPIO_WritePin(RF_CLEAR_GPIO_Port, RF_CLEAR_Pin, GPIO_PIN_RESET);

#define	ENABLE_RF_POWER						HAL_GPIO_WritePin(RF_POWER_CON_GPIO_Port, RF_POWER_CON_Pin, GPIO_PIN_SET);
#define	DISENABLE_RF_POWER					HAL_GPIO_WritePin(RF_POWER_CON_GPIO_Port, RF_POWER_CON_Pin, GPIO_PIN_RESET);

#define	ENABLE_VCCAD						HAL_GPIO_WritePin(VCCAD_CON_ON_OR_OFF_GPIO_Port, VCCAD_CON_ON_OR_OFF_Pin, GPIO_PIN_SET);
#define	DISENABLE_VCCAD						HAL_GPIO_WritePin(VCCAD_CON_ON_OR_OFF_GPIO_Port, VCCAD_CON_ON_OR_OFF_Pin, GPIO_PIN_RESET);

#define	ENABLE_PMICEN1						HAL_GPIO_WritePin(PMICEN1_GPIO_Port, PMICEN1_Pin, GPIO_PIN_SET);
#define	DISENABLE_PMICEN1					HAL_GPIO_WritePin(PMICEN1_GPIO_Port, PMICEN1_Pin, GPIO_PIN_RESET);

#define	FREQ_CON_SET						HAL_GPIO_WritePin(RF_FREQ_CON_GPIO_Port, RF_FREQ_CON_Pin, GPIO_PIN_SET);
#define	FREQ_CON_RESET						HAL_GPIO_WritePin(RF_FREQ_CON_GPIO_Port, RF_FREQ_CON_Pin, GPIO_PIN_RESET);

#define	DIR_485B_TX						HAL_GPIO_WritePin(DIR_485B_GPIO_Port, DIR_485B_Pin, GPIO_PIN_SET);
#define	DIR_485B_RX						HAL_GPIO_WritePin(DIR_485B_GPIO_Port, DIR_485B_Pin, GPIO_PIN_RESET);
//-------------------------------------------------------------------------------------------------------------------------------------------------
#define	MAXRANGE_70						GPIO_PIN_RESET
#define	MAXRANGE_35						GPIO_PIN_SET

#define	RF_FREQUENCY_26G					GPIO_PIN_RESET
#define	RF_FREQUENCY_6G						GPIO_PIN_SET

#define REBOOTCURRENT						20
#define UPDATECODECURRENT					26
#define LCDCURVE_LENGH						100

#define MAXCURRENT_SET						24
#define MINCURRENT_SET						2

#define ENABLEUPDATECODE_1443					0x5A
#define DISABLEUPDATECODE_1443					0

#define	SAMPLE_DATA_LENGTH					860//7000
#define	FAKEECHO_MAXVALURE					33000
/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
