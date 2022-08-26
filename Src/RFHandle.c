//#include	"stm32l4xx_hal.h"
#include	"main.h"
#include	"tim.h"
#include	"adc.h"
#include	"RFHandle.h"
#include	"EchoHandle.h"

#define	PERIOD_CONTROL_FAST		0
#define PERIOD_CONTROL_FAST1		1
#define PERIOD_CONTROL_SLOW		2

uint8_t  swtich1=0;
uint8_t  swtich2=0;
uint8_t  swtich3=0;
uint8_t  swtich4=0;

uint8_t	IntCount,DelayRFSignalSendCount,ADSampleCount;
uint8_t FreqConStatus;
uint8_t FreqLowCounter;
uint8_t YU_value;
uint32_t ErrorTime;
uint32_t CurrentPeriod;
uint32_t CenterPeriod;

float BiliXishu=0.005;
float JifenXishu=0.0008;
float P_Limit=100;
float I_Limit=50;
float PI_Limit=2000;
float BiasTimeSum_Limit = 100;
float I_Control;
float PI_Control;
float BiasTimeSum;

void	ConDelay(float DelayCount)
{
	do
  	{
		DelayCount--;
	}
	while(DelayCount > 0);
}

void	PeriodControl(uint32_t PeriodTime)
{
	float P_Control=0;
	float Control;
	float SinControl;
	float BiasTime=0;
	float temp2,temp3;

//	TurnLow();
	temp2 = CurrentPeriod;
	temp3 = CenterPeriod;
	BiasTime = temp2-temp3;
	BiasTimeSum = BiasTimeSum+BiasTime;
	//RF_DIGI_UP;
	P_Control=BiasTime*BiliXishu;
	
	if(BiasTimeSum > BiasTimeSum_Limit)BiasTimeSum = BiasTimeSum_Limit;
	if(BiasTimeSum < -BiasTimeSum_Limit)BiasTimeSum = -BiasTimeSum_Limit;
	I_Control = BiasTimeSum*JifenXishu;
	if(P_Control > 0)
	{	
		if(P_Control > P_Limit)P_Control=P_Limit;
	}
	else
	{
		if(P_Control < -P_Limit)P_Control=0-P_Limit;
	}
	if(I_Control>0)
	{
		if(I_Control > I_Limit)I_Control=I_Limit;
	}
	else
	{
		if(I_Control < -I_Limit)I_Control=0-I_Limit;
	}
	//RF_DIGI_UP;
	PI_Control=P_Control+I_Control;//40us
	SinControl=PI_Control;

	if(SinControl > PI_Limit)SinControl = PI_Limit;
	if(SinControl < -PI_Limit)SinControl = -PI_Limit;
	
	if(SinControl > 0)
	{
		Control = SinControl;
	}
	else
	{
		Control = 0-SinControl;
	}

	if( (SinControl > -1)&&( SinControl<1) )
	{
		RFSWITCHOFF;
	}
	else if(SinControl<=-1	)
	{
		if(Control == 1)
		{
			RFTURNHIGH;
			RFSWITCHOFF;
		}
		else
		{
			RFTURNHIGH;
			ConDelay(Control);
			RFSWITCHOFF;
		}
	}
	else if(SinControl >= 1)
	{
		if(Control == 1)
		{
			RFTURNLOW;
			RFSWITCHOFF;
		}
		else
		{
			RFTURNLOW;
			ConDelay(Control);
			RFSWITCHOFF;
		}
	}
}

void	DetectError(uint32_t PeriodTime)
{
	uint32_t TempIL;
	if(ErrorTime > ERRORUPLIMIT)
	{
		ErrorTime=0;
		RFTURNLOW;
		FreqConStatus = FREQ_START;
		return;
	}
	
	TempIL = CenterPeriod*2;
	 if(PeriodTime > TempIL)
	{
		ErrorTime = ERRORUPLIMIT;
	}
	 else 
	 {
	 	switch (YU_value)
	 	{
			case DizzyYU_P1: 
				if(ErrorTime>30)ErrorTime=ErrorTime-30;
				break;
			case DizzyYU_P2: ErrorTime++;
				break;
			case DizzyYU_P3: ErrorTime++;
				break;
			case DizzyYU_P4: ErrorTime++;
				break;
			case DizzyYU_P5: ErrorTime=ErrorTime+3;
				break;
			case DizzyYU_N1:  if(ErrorTime>30)ErrorTime=ErrorTime-30;
				break;
			case DizzyYU_N2: ErrorTime++;
				break;
			case DizzyYU_N3: ErrorTime++;
				break;
			case DizzyYU_N4: ErrorTime++;
				break;
			case DizzyYU_N5: ErrorTime=ErrorTime+3;
				break;
			default:break;
	 	}		
	}
}

void	PeriodWork(uint32_t PeriodTime)
{
	if(FreqConStatus == FREQ_CONTROL)
		PeriodControl(PeriodTime);
	else
	{
		FreqLowCounter++; 
		if(FreqLowCounter > LOW_TIME)
		{
			FreqConStatus = FREQ_CONTROL;
			FreqLowCounter = 0;
			RFSWITCHOFF;
		}
	}
}

void	GetRFCurrentPeriod(uint32_t TempPeriod)
{
	CurrentPeriod = TempPeriod;
}

void	PeriodProcess(void)
{
	PeriodWork(CurrentPeriod);			//this function can run for 2ms
	DetectError(CurrentPeriod);
//	ClearPeriod();
}
uint8_t GetADSampleCount(void)
{
	return(ADSampleCount);
}

void	ResetADSampleCount(void)
{
	ADSampleCount = 0;
}

void	HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM7)
	{
		StopTim7Delay();
		switch(DelayRFSignalSendCount)
		{
			case 0:
//				ENABLE_RF_POWER;
//				ENABLE_5V;
//				ENABLE_VCCAD;
//				ConfigureVref();
//				RF_SIGNAL_RESET;
				DelayRFSignalSendCount++;
				SetTim7Delay(STARTONEMSDELAY);
			break;
			case 1:
				RF_SIGNAL_RESET;
				SetTim7Delay(STARTONEMSDELAY);
				DelayRFSignalSendCount++;
			break;
			case 2:
				RF_SIGNAL_SET;
				StartSampleEcho();
//				ADC1_Start_DMA(LowSampleBuffer.LowSampleBuffer_32BIT, SAMPLE_DATA_LENGTH*2);
				DelayRFSignalSendCount++;
				ADSampleCount++;
			break;
			default:break;
		}
	}
}

void	ContralSampleRadio(void)
{
	IntCount++;
	switch(IntCount)
	{
		case ECHOSAMPLEWAIT:
			IntCount = STARTSAMPLE;
			DelayRFSignalSendCount = 0;
			SetTim7Delay(STARTRFVCCADDELAY);	//TIM7COUNTERCLOCK*13/1000
		break;
		case 2:
			EndSampleEcho();
//			ADC1_Stop_DMA();
		break;
		case 5:
//			SetCalDistanceFlag();
		break;
		default:break;
	}
}