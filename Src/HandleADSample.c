/* Includes ------------------------------------------------------------------*/
#include "tim.h"
#include "adc.h"
#include "gpio.h"
#include "dma.h"
#include "IWR1443.h"
#include "HandleADSample.h"
#include "perf_management.h"
#include "FRAM.h"
#include	"Eeprom.h"

VCON_CAP_ADSampleBuffer__TypeDef VCON_CAP_ADSampleBuffer;
MultiChannel_ADSampleBuffer__TypeDef MultiChannel_ADSampleBuffer;

uint8_t VCON_CAP_OR_MultiChannel_Sample;
uint8_t VCON_CAP_SampleStatus;
uint8_t MultiChannel_SampleStatus;
uint8_t StartRecord_MeasureValure_Flag;
uint16_t SamplPeriodCount;
uint32_t VCON_CAP_SampleStartTime;
uint32_t MultiChannel_SampleStartTime;
uint8_t InitPowerCount;

RecordMeasureValure__TypeDef RecordMeasureValure_CAP;
RecordMeasureValure__TypeDef RecordMeasureValure_Mul[CHANNEL_NUMBERS];
float MeasureValure_CAP_StartIWR1443;
float VCON_Threshold_Voltage;
void	ClearMeasureValure_Max_Min(void)
{
//	uint8_t TempC;
//	for(TempC = 0; TempC < CHANNEL_NUMBERS; TempC++)
//	{
//		RecordMeasureValure_Mul[TempC].MeasureValure_Max = 0;
//		RecordMeasureValure_Mul[TempC].MeasureValure_Min = 888888;
//	}
//	RecordMeasureValure_CAP.MeasureValure_Max = 0;
//	RecordMeasureValure_CAP.MeasureValure_Min = 888888;
}

void	InitADSamplePara(void)
{
	VCON_CAP_OR_MultiChannel_Sample = VCON_CAP_SAMPLE;
	VCON_CAP_SampleStatus = SAMPLEWAIT;
	MultiChannel_SampleStatus = SAMPLEWAIT;
	VCON_CAP_SampleStartTime = 0;
	MultiChannel_SampleStartTime = 0;
	StartRecord_MeasureValure_Flag = DISABLERECORD;
	RecordMeasureValure_Mul[7].MeasureValure_Current = 20;
//	ClearMeasureValure_Max_Min();
//	PerfDataBuffInit_Max_Min();
}

void	RecordMeasureValureMax_Min(void)
{
	uint8_t TempC;
//	float TempF;
	if(StartRecord_MeasureValure_Flag == ENABLERECORD)
	{
		StartRecord_MeasureValure_Flag = DISABLERECORD;
		RecordMeasureValure_Mul[AD_CHANNEL_NUMBERS].MeasureValure_Current = 1000*(RecordMeasureValure_Mul[1].MeasureValure_Current-RecordMeasureValure_Mul[0].MeasureValure_Current)/3.9;//CURRENT_CHANNEL_RES;
		for(TempC = 0; TempC < CHANNEL_NUMBERS; TempC++)
		{
/*			if(RecordMeasureValure_Mul[TempC].MeasureValure_Current > RecordMeasureValure_Mul[TempC].MeasureValure_Max)
			{
				RecordMeasureValure_Mul[TempC].MeasureValure_Max = RecordMeasureValure_Mul[TempC].MeasureValure_Current;
			}
			if(RecordMeasureValure_Mul[TempC].MeasureValure_Current < RecordMeasureValure_Mul[TempC].MeasureValure_Min)
			{
				RecordMeasureValure_Mul[TempC].MeasureValure_Min = RecordMeasureValure_Mul[TempC].MeasureValure_Current;
			}
*/			UpdatePerfdata_Max_Min(TempC*3+VOLAGECURRENT_STARTINDEX,RecordMeasureValure_Mul[TempC].MeasureValure_Current);
		}
		Record_Max_MinToFRAM();
	}
}

void	MX_ADC2_Init(void)
{
	ADC_ChannelConfTypeDef sConfig;
	
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	if(VCON_CAP_OR_MultiChannel_Sample == VCON_CAP_SAMPLE)
	{
		hadc1.Init.NbrOfConversion = 3;
	}
	else if(VCON_CAP_OR_MultiChannel_Sample == MULTICHANNEL_SAMPLE)
	{
		hadc1.Init.NbrOfConversion = AD_CHANNEL_NUMBERS;
	}
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.NbrOfDiscConversion = 1;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T4_CC4;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc1.Init.OversamplingMode = DISABLE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if(VCON_CAP_OR_MultiChannel_Sample == VCON_CAP_SAMPLE)
	{
		sConfig.Channel = ADC_CHANNEL_8;//ADC_CHANNEL_VREFINT;//
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.OffsetNumber = ADC_OFFSET_NONE;
		sConfig.Offset = 0;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		
		sConfig.Channel = ADC_CHANNEL_11;
		sConfig.Rank = 2;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		
		sConfig.Channel = ADC_CHANNEL_9;
		sConfig.Rank = 3;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	}
	else if(VCON_CAP_OR_MultiChannel_Sample == MULTICHANNEL_SAMPLE)
	{
		sConfig.Channel = ADC_CHANNEL_2;
		sConfig.Rank = 1;
		sConfig.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.OffsetNumber = ADC_OFFSET_NONE;
		sConfig.Offset = 0;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}

		sConfig.Channel = ADC_CHANNEL_3;
		sConfig.Rank 	= 2;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}

		sConfig.Channel = ADC_CHANNEL_16;
		sConfig.Rank = 3;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}

		sConfig.Channel = ADC_CHANNEL_14;
		sConfig.Rank = 4;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	
		sConfig.Channel = ADC_CHANNEL_15;
		sConfig.Rank = 5;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	
		sConfig.Channel = ADC_CHANNEL_10;
		sConfig.Rank = 6;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
		
/*		sConfig.Channel = ADC_CHANNEL_9;//ADC_CHANNEL_13;
		sConfig.Rank = 7;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}

		sConfig.Channel = ADC_CHANNEL_11;
		sConfig.Rank = 8;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
*/
		sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;//ADC_CHANNEL_9;
		sConfig.Rank = 7;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}

		sConfig.Channel = ADC_CHANNEL_VREFINT;
		sConfig.Rank = 8;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
	}
}

void	ADC2_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	if(VCON_CAP_OR_MultiChannel_Sample == VCON_CAP_SAMPLE)
	{
		GPIO_InitStruct.Pin = C4_20mAIn_Pin|VIN_CPU_Pin|VCON_CAP_Pin;//|GPIO_PIN_7;//
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else if(VCON_CAP_OR_MultiChannel_Sample == MULTICHANNEL_SAMPLE)
	{
		GPIO_InitStruct.Pin = VCC33_PW_Pin|VCC33_CPU_Pin|RF_POWER_13_Pin|RF_POWER_18_Pin;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RF_POWER_12_Pin|RF_POWER_22_Pin;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = VCC_25_Pin|VIN_CPU_Pin;//|C4_20mAIn_Pin;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

void	DisableVref(void)
{
	HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_ENABLE);
//	HAL_SYSCFG_VREFBUF_VoltageScalingConfig(SYSCFG_VREFBUF_VOLTAGE_SCALE1);
	HAL_SYSCFG_DisableVREFBUF();
}

void	ConfigureVref(void)
{
	HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE);
	HAL_SYSCFG_VREFBUF_VoltageScalingConfig(SYSCFG_VREFBUF_VOLTAGE_SCALE1);
	HAL_SYSCFG_EnableVREFBUF();
}

void	StartSampleClock(void)
{
	__HAL_RCC_TIM4_CLK_ENABLE();
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
}

void	StopSampleClock(void)
{
	__HAL_RCC_TIM4_CLK_DISABLE();
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
}

void	IncSamplePeriod(void)
{
	SamplPeriodCount++;
}

void	SetSampleStatus_Complete(void)
{
	DisableVref();
	StopSampleClock();
	HAL_ADC_Stop_DMA(&hadc1);
	HAL_ADC_DeInit(&hadc1);
	if(VCON_CAP_OR_MultiChannel_Sample == VCON_CAP_SAMPLE)
	{
		VCON_CAP_SampleStatus = SAMPLECOMPLETE;
	}
	else if(VCON_CAP_OR_MultiChannel_Sample == MULTICHANNEL_SAMPLE)
	{
		MultiChannel_SampleStatus = SAMPLECOMPLETE;
	}
}

void	StartSample_MultiChannel(void)
{
	VCON_CAP_OR_MultiChannel_Sample = MULTICHANNEL_SAMPLE;
	MX_ADC2_Init();
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, MultiChannel_ADSampleBuffer.ADSampleBuffer_32BIT, 2*MUL_ADSAMPLELENGTH);
	StartSampleClock();
}

void	StartSample_VCON_CAP(void)
{
	VCON_CAP_OR_MultiChannel_Sample = VCON_CAP_SAMPLE;
	MX_ADC2_Init();
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, VCON_CAP_ADSampleBuffer.ADSampleBuffer_32BIT, 2*VCON_CAP_ADSAMPLELENGTH);
	StartSampleClock();
}

void	MultiChannel_ConvertADSample(void)
{
	uint8_t TempC;
	uint16_t TempI;
	float TempF[AD_CHANNEL_NUMBERS];
	for(TempC = 0; TempC < AD_CHANNEL_NUMBERS; TempC++)
	{
		TempF[TempC] = 0;
	}
	for(TempI = AD_CHANNEL_NUMBERS; TempI < MUL_ADSAMPLELENGTH; TempI = TempI+AD_CHANNEL_NUMBERS)
	{
		for(TempC = 0; TempC < AD_CHANNEL_NUMBERS; TempC++)
		{
			TempF[TempC] = MultiChannel_ADSampleBuffer.ADSampleBuffer_16BIT[TempI+TempC]+TempF[TempC];
		}
	}
	for(TempC = 0; TempC < AD_CHANNEL_NUMBERS; TempC++)
	{
		RecordMeasureValure_Mul[TempC].MeasureValure_Current = (TempF[TempC]*AD_VREF)/AD_FULLADRESULT/(MUL_ADSAMPLELENGTH/AD_CHANNEL_NUMBERS-1);
		if((RecordMeasureValure_Mul[TempC].MeasureValure_Current < IWR1443_MINVOLTAGE)&&(TempC < 6))
		{
			AbnormalCloseWR1443Vcc();
//			TestCount++;
		}
	}
	RecordMeasureValure_Mul[0].MeasureValure_Current = RecordMeasureValure_Mul[0].MeasureValure_Current*VCC_33_GAIN;
	RecordMeasureValure_Mul[1].MeasureValure_Current = RecordMeasureValure_Mul[1].MeasureValure_Current*VCC_33_GAIN;
	TempF[0] = CAL_AVG_SLOPE;
	TempF[1] = TS_CAL1_V;
	RecordMeasureValure_Mul[6].MeasureValure_Current = 100*(1000*RecordMeasureValure_Mul[6].MeasureValure_Current-TempF[1])/TempF[0]+TEMPSENSOR_CAL1_TEMP;
	StartRecord_MeasureValure_Flag = ENABLERECORD;
}

void	VCON_CAP_ConvertADSample(void)
{
	uint8_t TempC;
	uint16_t TempI;
	float TempF[3];
	float TempExternalVoltage;
	TempF[0] = 0;
	TempF[1] = 0;
	for(TempI = 0; TempI < VCON_CAP_ADSAMPLELENGTH; TempI = TempI+3 )
	{
		for(TempC = 0; TempC < 3; TempC++)
		{
			TempF[TempC] = VCON_CAP_ADSampleBuffer.ADSampleBuffer_16BIT[TempI+TempC]+TempF[TempC];
		}
	}
	for(TempC = 0; TempC < 3; TempC++)
	{
		TempF[TempC] = (TempF[TempC]*AD_VREF)/(VCON_CAP_ADSAMPLELENGTH/3)/AD_FULLADRESULT;//mA
	}
	RecordMeasureValure_CAP.MeasureValure_Current = VCON_CAP_GAIN*TempF[0];
	TempF[1] = AD_VREF-2*TempF[1];
	TempExternalVoltage = VIN_GAIN*TempF[2]+TempF[1];
//	TempExternalCurrent = 10*(AD_VREF-TempF[1]);
	
//	TempExternalVoltage = (VIN_GAIN*TempF[1]*AD_VREF)/(VCON_CAP_ADSAMPLELENGTH/2)/AD_FULLADRESULT;
	
	UpdatePerfdata_Max_Min(EXTERNALCURRENT_INDEX,10*TempF[1]);
	UpdatePerfdata_Max_Min(EXTERNALVOLTAGE_INDEX,TempExternalVoltage);
	UpdatePerfdata_Max_Min(VCONVOLAGE_INDEX,RecordMeasureValure_CAP.MeasureValure_Current);
	
//	VCON_Threshold_Voltage = (EXTERNALCURRENT_MAX-TempExternalCurrent)*(VCON_CAP_MAX_VOLTAGE-VCON_CAP_MIN_VOLTAGE)
//	  /(EXTERNALCURRENT_MAX-EXTERNALCURRENT_MIN)+VCON_CAP_MIN_VOLTAGE;
	
	if(TempExternalVoltage < VCON_CAP_MIN_VOLTAGE)
	{
		VCON_Threshold_Voltage = GetVconMinVolage();//VCON_THRESHOLD_MIN_VOLTAGE;
	}
	else
	{
		if(TempF[1] > 1.2)
		{
			if(TempExternalVoltage >= 18)
				VCON_Threshold_Voltage = TempExternalVoltage-4.5;//VCON_THRESHOLD_MIN_VOLTAGE;
			else if(TempExternalVoltage >= 16)
				VCON_Threshold_Voltage = TempExternalVoltage-4;//VCON_THRESHOLD_MIN_VOLTAGE;
			else
				VCON_Threshold_Voltage = TempExternalVoltage-3.5;
		}
		else
		{
			VCON_Threshold_Voltage = TempExternalVoltage-3.5;
		}
		if(VCON_Threshold_Voltage < GetVconMinVolage())//VCON_THRESHOLD_MIN_VOLTAGE)
		{
			VCON_Threshold_Voltage = GetVconMinVolage();//VCON_THRESHOLD_MIN_VOLTAGE;
		}
	}
	
	if(((MeasureValure_CAP_StartIWR1443 > VCON_Threshold_Voltage)||(RecordMeasureValure_CAP.MeasureValure_Current > VCON_CAP_MAX_VOLTAGE))&&
	   (MeasureValure_CAP_StartIWR1443 > RecordMeasureValure_CAP.MeasureValure_Current))
	{
		UpdatePerfdata_Max_Min(VCONVOLAGE_DV_INDEX,(MeasureValure_CAP_StartIWR1443-RecordMeasureValure_CAP.MeasureValure_Current));
		MeasureValure_CAP_StartIWR1443 = 0;
	}

	if((RecordMeasureValure_CAP.MeasureValure_Current > VCON_Threshold_Voltage)
	   ||(RecordMeasureValure_CAP.MeasureValure_Current > VCON_CAP_MAX_VOLTAGE))
	{
		MeasureValure_CAP_StartIWR1443 = RecordMeasureValure_CAP.MeasureValure_Current;
		if(InitPowerCount > INITPOWER_COUNT)
		{
			InitPowerCount = INITPOWER_COUNT+1;
			Set_IWR1443_Status(IWR1443_START_POWERON);
		}
		else
		{
			InitPowerCount++;
			if(RecordMeasureValure_CAP.MeasureValure_Current > VCON_CAP_MAX_VOLTAGE)
			{
				Set_IWR1443_Status(IWR1443_START_POWERON);
			}
		}
	}
	else
	{
		VCON_CAP_OR_MultiChannel_Sample = VCON_CAP_SAMPLE;
	}
}

/*??????????????????*/
void	MultiChannel_StartSample(void)
{
	if(((HAL_GetTick()-MultiChannel_SampleStartTime) > MULTISAMPLEPERIOD)&&(MultiChannel_SampleStatus == SAMPLEWAIT))
	{
//		ConfigureVref();
//		SamplPeriodCount = 0;
		StartSample_MultiChannel();
		MultiChannel_SampleStartTime = HAL_GetTick();
		MultiChannel_SampleStatus = SAMPLESTART;
	}
}

/*??????????????????*/
void	VCON_CAP_SampleFlow(void)
{
	if(((HAL_GetTick()-VCON_CAP_SampleStartTime) > SAMPLEPERIOD)&&(VCON_CAP_SampleStatus == SAMPLEWAIT))
	{
//		ConfigureVref();
		SamplPeriodCount = 0;
		StartSample_VCON_CAP();
		VCON_CAP_SampleStartTime = HAL_GetTick();
		VCON_CAP_SampleStatus = SAMPLESTART;
	}
	else if(VCON_CAP_SampleStatus == SAMPLECOMPLETE)
	{
		VCON_CAP_OR_MultiChannel_Sample = MULTICHANNEL_SAMPLE;
		VCON_CAP_ConvertADSample();
		VCON_CAP_SampleStatus = SAMPLEWAIT;
	}	
}

void	HandleADSample(void)
{
	if(Get_IWR1443_Status() == IWR1443_WAIT_STATUS)
	{
		if(VCON_CAP_OR_MultiChannel_Sample == MULTICHANNEL_SAMPLE)
		{
			if(MultiChannel_SampleStatus == SAMPLECOMPLETE)
			{
				VCON_CAP_OR_MultiChannel_Sample = VCON_CAP_SAMPLE;
				MultiChannel_ConvertADSample();
				MultiChannel_SampleStatus = SAMPLEWAIT;
			}
		//	MultiChannel_SampleFlow();
		}
		if(MultiChannel_SampleStatus == SAMPLEWAIT)
		{
			VCON_CAP_SampleFlow();
		}
	}
}
