#include	"main.h"
#include	"gpio.h"
#include 	"DistanceDataHandle.h"
#include	"EchoHandle.h"

//--------------------------------------------------------------------------
extern uint32_t	CenterPeriod;
//--------------------------------------------------------------------------
void	CheckPort(void)
{
//	RF_SIGNAL_SET;
	ENABLE_RF_POWER;
	Init6GInnerDistanceCoefficient();
	Init_26GEchoJudgeData();
	if((MAXRANGE) == MAXRANGE_70)
	{
//			RF_MAXRANGE_70;
		if((RFFREQUENCY) == RF_FREQUENCY_6G)
		{
			CenterPeriod = (uint32_t)(PERIODCENTER_6G);
		}
		else if((RFFREQUENCY) == RF_FREQUENCY_26G)
		{
			CenterPeriod = (uint32_t)(PERIODCENTER_26G);
		}
	}
	else
	{
//		RF_MAXRANGE_35;
		if((RFFREQUENCY) == RF_FREQUENCY_6G)
		{
			CenterPeriod = (uint32_t)(PERIODCENTER_6G);
		}
		else if((RFFREQUENCY) == RF_FREQUENCY_26G)
		{
			CenterPeriod = (uint32_t)(PERIODCENTER_26G);
			Init26GInnerDistanceCoefficient();
			Init_26GEchoJudgeData();
		}
	}
}

void	InitPort(void)
{
//	ENABLE_5V;
	HART_RESET_SET;
	CheckPort();
	ParaConvert();
//	ENABLE_RF_VCC;
}
