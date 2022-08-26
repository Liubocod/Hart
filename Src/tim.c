/**
  ******************************************************************************
  * File Name          : TIM.c
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
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
#include	"tim.h"
#include	"gpio.h"
#include	"main.h"
#include	"RFHandle.h"
#include	"adc.h"

/* USER CODE BEGIN 0 */
/* Captured Values */
uint32_t               uwIC2Value1 = 0;
uint32_t               uwIC2Value2 = 0;
uint32_t               uwDiffCapture = 0;

/* Capture index */
uint16_t               uhCaptureIndex = 0;

/* Frequency Value */
float	RfFrequency = 0;

/* USER CODE END 0 */

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim8;

/* TIM1 init function */
void	MX_TIM1_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_SlaveConfigTypeDef sSlaveConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = TIM1_PRESCALERVALUE;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = TIM1_PERIOD_VALUE;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_OC_Init(&htim1) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
	sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
	sSlaveConfig.TriggerFilter = 0;
	if (HAL_TIM_SlaveConfigSynchronization(&htim1, &sSlaveConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = TIM1_PULSE_VALUE;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	HAL_TIM_MspPostInit(&htim1);

	/* Start the Output Compare */
	assert_param(IS_TIM_CCX_INSTANCE(htim1.Instance, TIM_CHANNEL_1));

	/* Enable the Output compare channel */
	if(HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
	{
		/* Start Error */
		_Error_Handler(__FILE__, __LINE__);
	}
	/* ### - 4 - Start conversion in DMA mode ################################# */
}
/* TIM2 init function */
void	MX_TIM2_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_OnePulse_InitTypeDef sConfig;
	TIM_MasterConfigTypeDef  sMasterConfig;

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.Period = 0x0003A980;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.RepetitionCounter = 0;

	if (HAL_TIM_OnePulse_Init(&htim2, TIM_OPMODE_SINGLE) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	
	/*##-2- Configure the Channel 1 in One Pulse mode ##########################*/
	sConfig.OCMode       = TIM_OCMODE_PWM2;
	sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
	sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	sConfig.Pulse        = 32000;
	sConfig.ICPolarity   = TIM_ICPOLARITY_FALLING;
	sConfig.ICSelection  = TIM_ICSELECTION_DIRECTTI;
	sConfig.ICFilter     = 0;

	if (HAL_TIM_OnePulse_ConfigChannel(&htim2, &sConfig, TIM_CHANNEL_2, TIM_CHANNEL_1) != HAL_OK)
	{
		/* Configuration Error */
		Error_Handler();
	}
	/* Configure TIM1 in Master Enable mode & use the update event as Trigger Output (TRGO) */
	sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_ENABLE;
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC2REF;
	if( HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		/* Configuration Error */
		Error_Handler();
	}  
	
	if(HAL_TIM_OnePulse_Start(&htim2, TIM_CHANNEL_2) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}
}
/* TIM3 init function */
void	MX_TIM3_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = TIM3_PRESCALERVALUE;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = TIM3_PERIOD_VALUE;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_OC_Init(&htim3) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = TIM3_PULSE_VALUE;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	HAL_TIM_MspPostInit(&htim3);
	if(HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2) != HAL_OK)
	{
		/* Starting Error */
		_Error_Handler(__FILE__, __LINE__);
	}
}
/* TIM4 init function */
void	MX_TIM4_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = TIM4_PRESCALERVALUE;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = TIM4_PERIOD_VALUE;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.RepetitionCounter = 0;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	sConfigOC.Pulse = TIM4_PULSE_VALUE;
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

//	HAL_TIM_MspPostInit(&htim2);
}

/* TIM5 init function */
void	MX_TIM5_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_IC_InitTypeDef sConfigIC;

	htim5.Instance = TIM5;
	htim5.Init.Prescaler = 0;
	htim5.Init.Period = 0xFFFFFFFF;
	htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim5.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_IC_Init(&htim5) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/*##-3- Start the Input Capture in interrupt mode ##########################*/
	if(HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_3) != HAL_OK)
	{
	/* Starting Error */
		Error_Handler();
	}
}

/* TIM6 init function */
void	MX_TIM6_Init(void)
{
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = TIM6_PRESCALERVALUE;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = TIM6_PERIOD_VALUE;
	htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}
}

/* TIM7 init function */
void	MX_TIM7_Init(void)
{
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = TIM7_PRESCALERVALUE;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = TIM7_PERIOD_VALUE;
	htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}
}

/* TIM8 init function */
void	MX_TIM8_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
//	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;
//	TIM_IC_InitTypeDef sConfigIC;
//	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

	htim8.Instance = TIM8;
	htim8.Init.Prescaler = TIM8_PRESCALERVALUE;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = TIM8_PERIOD_VALUE;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	/*##-2- Configure the PWM channels #########################################*/
	/* Common configuration for all channels */
	sConfigOC.OCMode       = TIM_OCMODE_PWM1;
	sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
	sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;

	/* Set the pulse value for channel 1 */
	sConfigOC.Pulse = TIM8_PULSE_VALUE;
	if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		/* Configuration Error */
		_Error_Handler(__FILE__, __LINE__);
	}

	HAL_TIM_MspPostInit(&htim8);
	/*##-3- Start PWM signals generation #######################################*/
	/* Start channel 1 */
	if (HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1) != HAL_OK)
	{
		/* PWM Generation Error */
		_Error_Handler(__FILE__, __LINE__);
	}
}

void	UpdateCurrentTime(uint32_t CurrentPWMValue)
{
	htim8.Instance->CCR1 = CurrentPWMValue;
}

void	HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef   GPIO_InitStruct;

	/* Enable the TIMx clock */
	__HAL_RCC_TIM2_CLK_ENABLE();

	/* Configure PA.00 and PB.03 */
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;

	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
  
    /**TIM1 GPIO Configuration    
    PE7     ------> TIM1_ETR 
    */
    GPIO_InitStruct.Pin = RF_FREQU_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(RF_FREQU_GPIO_Port, &GPIO_InitStruct);

    /* TIM1 interrupt Init */
//    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
  /* USER CODE BEGIN TIM1_MspInit 1 */

  /* USER CODE END TIM1_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
  
    /**TIM2 GPIO Configuration    
    PA0     ------> TIM2_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure PA.00 and PB.03 */
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;

  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM2_MspInit 1 */
    /* TIM2 interrupt Init */
//	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
//	HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* TIM3 interrupt Init */
//    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();
    /* USER CODE BEGIN TIM4_MspInit 1 */

  /* USER CODE END TIM4_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM5)
  {
  /* USER CODE BEGIN TIM5_MspInit 0 */

  /* USER CODE END TIM5_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM5_CLK_ENABLE();
  
    /**TIM5 GPIO Configuration    
    PA1     ------> TIM5_CH2
    PA2     ------> TIM5_CH3 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|MEASURE_FREQU_PERIOD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM5 interrupt Init */
	HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);
  /* USER CODE BEGIN TIM5_MspInit 1 */

  /* USER CODE END TIM5_MspInit 1 */
  }
	else if(tim_baseHandle->Instance==TIM6)
	{
		/*##-1- Enable peripherals and GPIO Clocks #################################*/
		/* TIMx Peripheral clock enable */
		__HAL_RCC_TIM6_CLK_ENABLE();
  
		/*##-2- Configure the NVIC for TIMx ########################################*/
		/* Set the TIMx priority */
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 4, 0);

		/* Enable the TIMx global Interrupt */
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	}
	else if(tim_baseHandle->Instance==TIM7)
	{
		/*##-1- Enable peripherals and GPIO Clocks #################################*/
		/* TIMx Peripheral clock enable */
		__HAL_RCC_TIM7_CLK_ENABLE();
  
		/*##-2- Configure the NVIC for TIMx ########################################*/
		/* Set the TIMx priority */
		HAL_NVIC_SetPriority(TIM7_IRQn, 3, 0);

		/* Enable the TIMx global Interrupt */
		HAL_NVIC_EnableIRQ(TIM7_IRQn);
	}
	else if(tim_baseHandle->Instance==TIM8)
	{
		/* Peripheral clock enable */
		__HAL_RCC_TIM8_CLK_ENABLE();
  
		/**TIM8 GPIO Configuration    
		PC7     ------> TIM8_CH2  */
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	}
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(timHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspPostInit 0 */

  /* USER CODE END TIM1_MspPostInit 0 */
    /**TIM1 GPIO Configuration    
    PE9     ------> TIM1_CH1 
    */
/*    GPIO_InitStruct.Pin = HSAMPLE_CLOCK_OUT_Pin|RF_FREQU_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(HSAMPLE_CLOCK_OUT_GPIO_Port, &GPIO_InitStruct);
*/
  /* USER CODE BEGIN TIM1_MspPostInit 1 */

  /* USER CODE END TIM1_MspPostInit 1 */
  }
  else if(timHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspPostInit 0 */

  /* USER CODE END TIM2_MspPostInit 0 */
  
    /**TIM2 GPIO Configuration    
    PB3 (JTDO-TRACESWO)     ------> TIM2_CH2 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM2_MspPostInit 1 */

  /* USER CODE END TIM2_MspPostInit 1 */
  }
  else if(timHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspPostInit 0 */

  /* USER CODE END TIM3_MspPostInit 0 */
  
    /**TIM3 GPIO Configuration    
    PB5     ------> TIM3_CH2 
    */
    GPIO_InitStruct.Pin = WD_CLOCK_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(WD_CLOCK_OUT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM3_MspPostInit 1 */

  /* USER CODE END TIM3_MspPostInit 1 */
  }
  else if(timHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspPostInit 0 */

  /* USER CODE END TIM4_MspPostInit 0 */
  
    /**TIM4 GPIO Configuration    
    PB9     ------> TIM4_CH4 
    */
    GPIO_InitStruct.Pin = LSAMPLE_CLOCK_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(LSAMPLE_CLOCK_OUT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM4_MspPostInit 1 */

  /* USER CODE END TIM4_MspPostInit 1 */
  }
  else if(timHandle->Instance==TIM8)
  {
  /* USER CODE BEGIN TIM8_MspPostInit 0 */

  /* USER CODE END TIM8_MspPostInit 0 */
  
    /**TIM8 GPIO Configuration    
    PC6     ------> TIM8_CH1 
    */
    GPIO_InitStruct.Pin = CURRENT_OUT_CON_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init(CURRENT_OUT_CON_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM8_MspPostInit 1 */

  /* USER CODE END TIM8_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();
  
    /**TIM1 GPIO Configuration    
    PE7     ------> TIM1_ETR
    PE9     ------> TIM1_CH1 
    */
//    HAL_GPIO_DeInit(GPIOE, RF_FREQU_Pin|HSAMPLE_CLOCK_OUT_Pin);

    /* TIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

  /* USER CODE END TIM1_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();
  
    /**TIM2 GPIO Configuration    
    PA0     ------> TIM2_CH1
    PB3 (JTDO-TRACESWO)     ------> TIM2_CH2 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);

  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();

    /* TIM3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspDeInit 0 */

  /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();
  
    /**TIM4 GPIO Configuration    
    PB9     ------> TIM4_CH4
    PE0     ------> TIM4_ETR 
    */
    HAL_GPIO_DeInit(LSAMPLE_CLOCK_OUT_GPIO_Port, LSAMPLE_CLOCK_OUT_Pin);
    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspDeInit 1 */

  /* USER CODE END TIM4_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM5)
  {
  /* USER CODE BEGIN TIM5_MspDeInit 0 */

  /* USER CODE END TIM5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM5_CLK_DISABLE();
  
    /**TIM5 GPIO Configuration    
    PA1     ------> TIM5_CH2
    PA2     ------> TIM5_CH3 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|MEASURE_FREQU_PERIOD_Pin);

    /* TIM5 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM5_IRQn);
  /* USER CODE BEGIN TIM5_MspDeInit 1 */

  /* USER CODE END TIM5_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM8)
  {
  /* USER CODE BEGIN TIM8_MspDeInit 0 */

  /* USER CODE END TIM8_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM8_CLK_DISABLE();
  
    /**TIM8 GPIO Configuration    
    PC6     ------> TIM8_CH1
    PC7     ------> TIM8_CH2 
    */
    HAL_GPIO_DeInit(GPIOC, CURRENT_OUT_CON_Pin|GPIO_PIN_7);

  /* USER CODE BEGIN TIM8_MspDeInit 1 */

  /* USER CODE END TIM8_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
void	HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
	{
		if(uhCaptureIndex == 0)
		{
			/* Get the 1st Input Capture value */
			uwIC2Value1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
			uhCaptureIndex = 1;
		}
		else if(uhCaptureIndex == 1)
		{
			/* Get the 2nd Input Capture value */
			uwIC2Value2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3); 

			/* Capture computation */
			if (uwIC2Value2 > uwIC2Value1)
			{
				uwDiffCapture = (uwIC2Value2 - uwIC2Value1); 
			}
			else if (uwIC2Value2 < uwIC2Value1)
			{
				/* 0xFFFF is max TIM5_CCRx value */
				uwDiffCapture = ((0xFFFFFFFF - uwIC2Value1) + uwIC2Value2) + 1;
			}
			else
			{
				/* If capture values are equal, we have reached the limit of frequency  measures */
				Error_Handler();
			}
			/* Frequency computation: for this example TIMx  is clocked by APB2Clk */      
			RfFrequency = HAL_RCC_GetPCLK2Freq()/1.0/uwDiffCapture;
			GetRFCurrentPeriod(uwDiffCapture);
			uhCaptureIndex = 0;
		}
	}
}

void	StartSampleEcho(void)
{
	__HAL_RCC_TIM1_CLK_ENABLE();			//start TIM1 clk
	htim1.Instance->SMCR |= 0x00000066;		//110: Filtered Timer Input 2 (TI2FP2)¡¢111: External Trigger input (ETRF) 
	htim1.Instance->CCER |= 0x00000001;
	htim1.Instance->BDTR |= (TIM_BDTR_MOE);
	//----------------start delay high sample-------------------------------
/*	HAL_TIM_OnePulse_Start(&htim2, TIM_CHANNEL_2);
	htim4.Instance->ARR = TIM4_PERIOD_VALUE_H;
	htim4.Instance->CCR4 = TIM4_PULSE_VALUE_H;
	htim4.Instance->SMCR |= 0x00000016;		//110: Filtered Timer Input 2 (TI2FP2)¡¢111: External Trigger input (ETRF) 
	htim4.Instance->CCER |= 0x00001000;		//
*///	htim4.Instance->BDTR |= (TIM_BDTR_MOE);
}

void	EndSampleEcho(void)
{
//	DISABLE_5V;
//	DISENABLE_VCCAD;
//	DISENABLE_RF_POWER;
//	DisableVref();

	HAL_TIM_OC_Stop(&htim1, TIM_CHANNEL_1);
	htim1.Instance->SMCR &= 0xFFFF0000; 
	__HAL_RCC_TIM1_CLK_DISABLE();			//stop TIM1 clk
	//-----------------stop delay high sample-------------------------------
/*	HAL_TIM_OnePulse_Stop(&htim2, TIM_CHANNEL_2);
	HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_4);
	htim4.Instance->SMCR &= 0xFFFF0000; 
*/
}

//-------------------------start delay sample flow------------------------------
void	SetTim7Delay(uint16_t DelayTime)
{
	__HAL_RCC_TIM7_CLK_ENABLE();
	htim7.Instance->ARR = DelayTime;
	__HAL_TIM_ENABLE(&htim7);
}

void	StopTim7Delay(void)
{
	__HAL_TIM_DISABLE(&htim7);
	__HAL_RCC_TIM7_CLK_DISABLE();
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
