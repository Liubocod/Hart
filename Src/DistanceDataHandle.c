/* -----------------------------------Includes --------------------------------------------------------*/
#include	"iwdg.h"
#include	"IWR1443.h"
#include	"tim.h"
#include	"usart.h"
#include	"Eeprom.h"
#include	"RFHandle.h"
#include	"EchoHandle.h"
#include 	"DistanceDataHandle.h"
#include	"CopyRightActivate.h"
#include	"perf_management.h"
#include	"BlueTooth_Com.h"
#include 	"HartHandle.h"
/*------------------------- External variables --------------------------------------------------------*/
extern	EepromParaU__TypeDef	EepromPara;
//-----------------------------------local variables----------------------------------------------------
MasterVariable__TypeDef MasterVariable;
float SystemDistanceAfterJump,OneSecondEvenValure,Qushi_Speed;
float SystemZeroDist,SystemProportion;
float PVEvenBuffe[PVEVEN_NUMBER_MAX];
float InnerOutDistance[5];
float OutputMS_Threshold,InputMS_Threshold;
float SystemDistanceOutCurrent = 0;
uint32_t lastpwm,CurrentPWM,StartSlowOut_Delay;
uint8_t ConfirmCounterJump,ConfirmCounterWaveLost,WorkStatus,MeterOutStatus;
uint16_t WaitCounter,RecordJump_WaitCounter;
uint8_t NoSampleSignal,QuickOutDistance_Flag;
uint16_t G_RevengeTimer;
//-----------------------------------------------------------------------------------------------------
float	CalcBiliBiaojiao(float TempPercent);
float	CalcJuliBiaojiao(float TempDistance);
//-----------------------------------------------------------------------------------------------------
/**void  RunActivationSystem()//函数名称和参量和返回值类型。
* 变量范围: 无 
*
* 返回值说明: 
*
* 功能介绍: 运行激活系统，该函数执行激活码的检查以及对检查结果的处理
*
* 关键备注：
*
* 未来提高:
* 
*/ 
void	RunActivationSystem(void)
{
	CheckActivationCode();
	G_RevengeTimer++;
	
	if(RevengeOrNot() == FALSE)
	{	         
		WorkStatus &= 0x7F; 
		G_RevengeTimer = 0;
	} 
	else
	{
		WorkStatus |= 0x80;
		//执行报复行为，定期使输出电流输出一次最大值
		if(G_RevengeTimer < REVENGEOUTTIME)
		{
			MasterVariable.MasterVariablePV = EepromPara.EepromParaS.RangeValue.LowerRange;
		}
		else if(G_RevengeTimer > REVENGEPERIOD)
		{
			G_RevengeTimer = 0;
		}
	} 
	if (Read_ActivateCodeErrorCount() != 0)
	{
		WorkStatus |= 0x40; 
	}
	else
	{
		WorkStatus &= 0xBF;   
	}
}

void	Init26GInnerDistanceCoefficient(void)
{
	SystemProportion = INNERPROPORTION_26G;
	SystemZeroDist = INNERZERO_26G;
	ConfirmCounterJump = 0x5A;
	WorkStatus = 0;
}

void	Init6GInnerDistanceCoefficient(void)
{
	SystemProportion = INNERPROPORTION_6G;
	SystemZeroDist = INNERZERO_6G;
	ConfirmCounterJump = 0x5A;
}

float	PointIndexIntToDistanceOut(uint16_t IndexPoint)
{
	float IndexFloat,SystemDistanceOut;
	IndexFloat = IndexPoint*OnePoint_M_Para;
	if(IndexFloat >= (SystemZeroDist+EepromPara.EepromParaS.DistCoefficient.SystemZeroDist))
	{
		SystemDistanceOut=((IndexFloat-EepromPara.EepromParaS.DistCoefficient.SystemZeroDist-SystemZeroDist)*EepromPara.EepromParaS.DistCoefficient.SystemProportion);
	}
	else SystemDistanceOut=0;
	return SystemDistanceOut;
}

uint16_t DistanceOutToPointIndexInt(float DistanceOut)
{
	uint16_t IndexPoint;
	float Index;
	Index = (DistanceOut+EepromPara.EepromParaS.DistCoefficient.SystemZeroDist+SystemZeroDist)/EepromPara.EepromParaS.DistCoefficient.SystemProportion/SystemProportion;
	if(Index > 0)
		IndexPoint = (uint16_t)(Index/OnePoint_M_Para);
	else
		IndexPoint = 0;
	if(IndexPoint > SAMPLE_DATA_LENGTH)
		IndexPoint = SAMPLE_DATA_LENGTH;
	return IndexPoint;
}

float	HartReadMasterVariable(uint8_t Index)
{
	float TempF;
	switch(Index)
	{
		case 0:
		TempF = (EepromPara.EepromParaS.RangeValue.LowerRange-MasterVariable.MasterVariablePV)*GetUnitConv_Ratio();
		break;
		case 1:
		TempF = MasterVariable.MasterVariablePV*GetUnitConv_Ratio();
		break;
		case 2:
		TempF = MasterVariable.MasterVariablePercent*100;
		break;
		case 3:
		TempF = MasterVariable.MasterVariablePVCurrent;
		break;
		case 4://temperature
		PerfDataRead(66,&TempF);
		break;
		default:break;
	}
	return(TempF);
}

void	ReadMMasterVariable(float *ParaValueP)
{
	*ParaValueP = MasterVariable.MasterVariablePV;
	ParaValueP++;
	*ParaValueP = MasterVariable.MasterVariablePercent;	
}

void	ReadMMasterVariablePVCurrent(float *ParaValueP)
{
	*ParaValueP = MasterVariable.MasterVariablePVCurrent;
}

uint8_t ReadMeterWorkStatus(void)
{
	return(WorkStatus);
}

float	CalcJuliBiaojiao(float Distance)
{
	uint8_t TempC=0;
	float DistanceTemp=Distance;
	float percent;
	
	DistanceTemp = Distance;
	for(TempC = 1; TempC < EepromPara.EepromParaS.LinearPara.LinePointsNum; TempC++) 
	{
		if( (EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC-1][0] <= Distance) && ( EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][0] >= Distance ) )
		{
			if(EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC-1][0] == EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][0]) return DistanceTemp;

			percent = (Distance-EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC-1][1])/(EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][1]-EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC-1][1]);

			DistanceTemp = percent*(EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][0]-EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC-1][0])+EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC-1][0];
			return(DistanceTemp);
		}
	}
	return(DistanceTemp);
}

void	QuickOutDistance(float TempDistance)
{
	if(QuickOutDistance_Flag != 0xA5)
	{
		QuickOutDistance_Flag = 0xA5;
		PVEvenBuffe[0] = TempDistance;
		MasterVariable.MasterVariablePV = TempDistance;
	}
}

void	CalcPercent(float Distance)
{	
	float percentage;
	if(Distance > EepromPara.EepromParaS.RangeValue.LowerRange)
	{
		WorkStatus |= 0x10;
		MeterOutStatus = 5;
		Distance = EepromPara.EepromParaS.RangeValue.LowerRange;
	}
	else
		WorkStatus &= 0xEF;
	if(Distance < EepromPara.EepromParaS.RangeValue.UpperRange)
	{
		WorkStatus |= 0x08;
		MeterOutStatus = 4;
		Distance = EepromPara.EepromParaS.RangeValue.UpperRange;
	}
	else
		WorkStatus &= 0xF7;
	if(EepromPara.EepromParaS.RangeValue.LowerRange != EepromPara.EepromParaS.RangeValue.UpperRange)
		percentage = (Distance-EepromPara.EepromParaS.RangeValue.LowerRange)/(EepromPara.EepromParaS.RangeValue.UpperRange-EepromPara.EepromParaS.RangeValue.LowerRange);
	if(percentage < 0)
		percentage = 0-percentage;
	if(percentage >= 1)
		percentage = 1;
	MasterVariable.MasterVariablePV = Distance;
	MasterVariable.MasterVariablePercent = percentage;
	UpdatePerfdata_Max_Min(REALTIMEMEASUREMENT_INDEX,MasterVariable.MasterVariablePV);
}

void	CurrentFloat(float current)
{	
	float current1mapwm=0;
	uint32_t TempLong;
	current1mapwm = EepromPara.EepromParaS.PWMCurrentPara.PWM4mA-EepromPara.EepromParaS.PWMCurrentPara.PWM18mA;
	current1mapwm = current1mapwm/14;
	if(current <= 18)
	{	
		TempLong = (uint32_t)((18-current)*current1mapwm+EepromPara.EepromParaS.PWMCurrentPara.PWM18mA);
		if(TempLong > 65530)
			CurrentPWM = 65530;
		else
			CurrentPWM = TempLong;
	}
	else
	{
		current1mapwm = (current-18)*current1mapwm;
		if(current1mapwm < EepromPara.EepromParaS.PWMCurrentPara.PWM18mA)
			CurrentPWM = (uint32_t)(EepromPara.EepromParaS.PWMCurrentPara.PWM18mA-current1mapwm);
		else
			CurrentPWM = 0;
	}
}

void	SetOutCurrentVal(float CurrentVal)
{
	MasterVariable.MasterVariableSetCurrent = CurrentVal;
}

void	CalcCurrent(void)
{
	if(EepromPara.EepromParaS.PWMCurrentPara.CurrentDirection == 0)
		MasterVariable.MasterVariablePVCurrent = MasterVariable.MasterVariablePercent*16+4;
	else
		MasterVariable.MasterVariablePVCurrent = 20-MasterVariable.MasterVariablePercent*16;
	if((WorkStatus != 4)&&(WorkStatus != 0)&&(MasterVariable.MasterVariableSetCurrent == 0))
	{
		if((WorkStatus != 0x02)||((WorkStatus == 0x02)&&(EepromPara.EepromParaS.WaveLostPara.WaveLostProccessSEL == 3)))
		{
			switch(EepromPara.EepromParaS.PWMCurrentPara.CurrentAlarmSEL)
			{
				case 1:
					CurrentFloat(3.5);
					break;
				case 2:
					CurrentFloat(20.5);
					break;
				case 3:
					CurrentFloat(22);
					break;
				default:
					if(MasterVariable.MasterVariableSetCurrent == 0)
					{
						if(EepromPara.EepromParaS.ComPara.ComAddress == 0)
							CurrentFloat(MasterVariable.MasterVariablePVCurrent);
						else
							CurrentFloat(4);			
					}
					else
						CurrentFloat(MasterVariable.MasterVariableSetCurrent);
					break;
			}
		}
		else
		{
			if(MasterVariable.MasterVariableSetCurrent == 0)
			{
				if(EepromPara.EepromParaS.ComPara.ComAddress == 0)
					CurrentFloat(MasterVariable.MasterVariablePVCurrent);
				else
					CurrentFloat(4);			
			}
			else
				CurrentFloat(MasterVariable.MasterVariableSetCurrent);
		}
	}
	else
	{
		if(MasterVariable.MasterVariableSetCurrent == 0)
		{
			if(EepromPara.EepromParaS.ComPara.ComAddress == 0)
				CurrentFloat(MasterVariable.MasterVariablePVCurrent);
			else
				CurrentFloat(4);			
		}
		else
			CurrentFloat(MasterVariable.MasterVariableSetCurrent);
	}
//	CurrentFloat(22);
}

void	UpdateCurrentPWM(void)
{
	uint32_t TempL;
	TempL = lastpwm;
	if(CurrentPWM != TempL)
	{
		if(CurrentPWM > lastpwm)
		{
                    lastpwm = lastpwm+10;
                    if(lastpwm > CurrentPWM)
                    {
                      lastpwm = CurrentPWM;
                    }
				
		}
		else
		{
                    //lastpwm = lastpwm-10;
                    //if(lastpwm < CurrentPWM)
                    lastpwm = CurrentPWM;
		}
		UpdateCurrentTime(lastpwm);
	}
	else
	{
		UpdateCurrentTime(lastpwm);
	}
}

void	SetUpdateCode_Current(void)
{
//	CurrentFloat(UPDATECODECURRENT);
	lastpwm = 0;
	CurrentPWM = 0;
	UpdateCurrentTime(lastpwm);
}

float	WaveLostProcess(float LastTimePV)
{
	float Distance;
	if((GetBestEchoIndex() == BEST_WAVE_NOT_FIND)&&(MeterOutStatus != METEROUTSTATUS_B))
	{
		if(ConfirmCounterWaveLost < 6)
		{
			ConfirmCounterWaveLost++;
			Distance = LastTimePV;
		}
		else
		{
			switch(EepromPara.EepromParaS.WaveLostPara.WaveLostProccessSEL)
			{
				case	MEANING_WaveLostProccessSel_BAOCHI:
				  	MeterOutStatus = 6;
					Distance = LastTimePV;
				break;
				case	MEANING_WaveLostProccessSel_ALARM:
				  	MeterOutStatus = 11;
					Distance = LastTimePV;
				break;
				case	MEANING_WaveLostProccessSel_SHEDING:
				  	MeterOutStatus = 7;
					Distance = EepromPara.EepromParaS.WaveLostPara.WaveLostSetPosition;
					MasterVariable.MasterVariablePV = EepromPara.EepromParaS.WaveLostPara.WaveLostSetPosition;
				break;
				case	MEANING_WaveLostProccessSel_QUSHIYIDONG:
				  	MeterOutStatus = 10;
/*					TempF = InnerOutDistance[0]-InnerOutDistance[9];
					if(TempF < -0.006)
					{
						if(SystemDistanceAfterWaveLost > HART_Variables.WaveLostSettedPosition)
							Distance = SystemDistanceAfterWaveLost-HART_Variables.WaveLostSettedPosition/2;
						else
							Distance = 0;
					}
					else if(TempF > 0.006)
					{
						Distance = SystemDistanceAfterWaveLost+HART_Variables.WaveLostSettedPosition/2;
						if(Distance > HART_Variables.PVLowerRangeValue)
							Distance = HART_Variables.PVLowerRangeValue;
					}*/
				break;
//				case	MEANING_WaveLostProccessSel_POWERREST:
//					Distance = HART_Variables.PVLowerRangeValue;
//				break;
				default:
					EepromPara.EepromParaS.WaveLostPara.WaveLostProccessSEL = MEANING_WaveLostProccessSel_BAOCHI;
				break;
			}
			ConfirmCounterWaveLost = 6;
		}
	}
	else
	{
		ConfirmCounterWaveLost = 0;
		Distance = LastTimePV;
	}
	return(Distance);
}

//uint32_t TempRunTime;
float	DistanceCalibration(float InputDistanceValure)
{
	float TempDistance;
//	TempRunTime = HAL_GetTick();
	if((InputDistanceValure <= 120) && (InputDistanceValure >= 0))
	{	
		TempDistance = InputDistanceValure*EepromPara.EepromParaS.DistCoefficient.SystemProportion*SystemProportion
		  -EepromPara.EepromParaS.DistCoefficient.SystemZeroDist-SystemZeroDist;
//		TempRunTime = TempRunTime-HAL_GetTick();
		if(TempDistance >= 0)
		{
			return(TempDistance);
		}
		else 
			return(0);
	}
	else
	{
		return(DISTANCEERROR);
	}
}

float	CalculateDistance(float SystemDistanceInner)
{
//	float SystemDistanceOut;
//	SystemDistanceOut = DistanceCalibration(SystemDistanceInner);
	if(EepromPara.EepromParaS.LinearPara.DLineUseorUnuse == MEANING_XIANXING_JULI_YONG)
	{
		return(CalcJuliBiaojiao(SystemDistanceInner));
	}
	else
	{
		return(SystemDistanceInner);
	}
}

void	HandleMeterStatus_R(void)
{
	float TempF;
	if(SystemDistanceAfterJump > MasterVariable.MasterVariablePV)
	{
		TempF = SystemDistanceAfterJump-MasterVariable.MasterVariablePV;
	}
	else
	{
		TempF = MasterVariable.MasterVariablePV-SystemDistanceAfterJump;
	}
	if(TempF >= 0.2)
	{
		if(MeterOutStatus == 0)
		{
			MeterOutStatus = 17;
		}
		else if(MeterOutStatus == 7)
		{
			MeterOutStatus = 20;			
		}
	}
}

//按照阻尼时间（以秒为单位）进行计算测量值平均值
float	CalculatePVEven(void)
{
	float TotalValue,MinTemp,MaxTemp;
	int16_t TempI;
	
	MinTemp = SystemDistanceAfterJump;
	MaxTemp = SystemDistanceAfterJump;

	//堆栈刷新，最旧的数据不要，把最新的数据放在第0号位
	HandleMeterStatus_R();
	TotalValue = SystemDistanceAfterJump;
	for(TempI = EepromPara.EepromParaS.BasicPara.DampingTime; TempI >= 0; TempI--)
	{
		TotalValue = TotalValue + PVEvenBuffe[TempI];
		if(TempI == 0)
		{
			PVEvenBuffe[0] = SystemDistanceAfterJump;  
		}
		else
		{
			PVEvenBuffe[TempI] = PVEvenBuffe[TempI-1];
		}
		//去掉最大值和最小值
		if(MaxTemp <= PVEvenBuffe[TempI])
		{
			MaxTemp = PVEvenBuffe[TempI];
		}
		if(MinTemp >= PVEvenBuffe[TempI])
		{
			MinTemp = PVEvenBuffe[TempI];
		}		
	}
	for(TempI = (PVEVEN_NUMBER_MAX-1); TempI > EepromPara.EepromParaS.BasicPara.DampingTime; TempI--)
	{
		PVEvenBuffe[TempI] = SystemDistanceAfterJump;
	}
	
	if(EepromPara.EepromParaS.BasicPara.DampingTime > 0)
	{
		return((TotalValue-MinTemp-MaxTemp)/EepromPara.EepromParaS.BasicPara.DampingTime);
	}
	else
	{
		return(SystemDistanceAfterJump);
	}	
}

void	SlowOut(float TempDistance)
{
	uint32_t TempL;
	float TempDV;
	StartSlowOut_Delay++;
	TempL = EepromPara.EepromParaS.ExpandPara.CharPara[15]*60;
	if((SystemDistanceOutCurrent+EepromPara.EepromParaS.ExpandPara.FloatPara[0] < 6000)&&(StartSlowOut_Delay >= TempL))
	{
		StartSlowOut_Delay = TempL;
		TempDV = TempDistance-InnerOutDistance[0];

		if(TempDV >= 0)//空高变大，为出料状态
		{
			if(TempDV > EepromPara.EepromParaS.ExpandPara.FloatPara[0])//OutputMS_Threshold)
			{
				SystemDistanceOutCurrent = SystemDistanceOutCurrent+EepromPara.EepromParaS.ExpandPara.FloatPara[0];
				if(MeterOutStatus == 0)
				{
					MeterOutStatus = 18;
				}
				else if(MeterOutStatus == 7)
				{
					MeterOutStatus = 21;
				}
				if(SystemDistanceOutCurrent > TempDistance)
					SystemDistanceOutCurrent = TempDistance;
			}
			else
			{
				SystemDistanceOutCurrent = TempDistance;
			}
		}
		else	//空高变小，为进料状态
		{
			TempDV = 0-TempDV;
			if(TempDV > EepromPara.EepromParaS.ExpandPara.FloatPara[1])//InputMS_Threshold)
			{
				if(SystemDistanceOutCurrent > EepromPara.EepromParaS.ExpandPara.FloatPara[1])
				{
					SystemDistanceOutCurrent = SystemDistanceOutCurrent-EepromPara.EepromParaS.ExpandPara.FloatPara[1];	//
					if(MeterOutStatus == 0)
					{
						MeterOutStatus = 18;
					}
					else if(MeterOutStatus == 7)
					{
						MeterOutStatus = 21;
					}
				}
				else
					SystemDistanceOutCurrent = TempDistance;
			}
			else
			{
				SystemDistanceOutCurrent = TempDistance;
			}
		}	
//		for(TempC = 3; TempC > 0; TempC--)
//			InnerOutDistance[TempC+1] = InnerOutDistance[TempC];
//		InnerOutDistance[1] = InnerOutDistance[0];
		InnerOutDistance[0] = SystemDistanceOutCurrent;
	}
	else
	{
		SystemDistanceOutCurrent = TempDistance;
	}
}

void	JumpProccess(float TempDistance)
{
	float D_value;
	uint16_t TempI;

	if(TempDistance > MasterVariable.MasterVariablePV)
		D_value = TempDistance-MasterVariable.MasterVariablePV;
	else
		D_value = MasterVariable.MasterVariablePV-TempDistance;

	if(D_value < EepromPara.EepromParaS.JumpPara.JumpDist)
	{
		SystemDistanceAfterJump = TempDistance;
		if(ConfirmCounterJump != 0x5A)//仪表上电测量距离不进行等待处理
		{
			ConfirmCounterJump = 0;
		}
		WaitCounter = 0;
	}
	else
	{
		if(ConfirmCounterJump == 0x5A)//仪表上电测量距离不进行等待处理
		{//初始化平均值缓冲区
			for(TempI = 0; TempI < PVEVEN_NUMBER_MAX; TempI++)
			{
				PVEvenBuffe[TempI] = TempDistance;
			}
		}
		ConfirmCounterJump++;
		if(ConfirmCounterJump < CONFIRMJUMP)
		{
			WaitCounter = (uint16_t)(EepromPara.EepromParaS.JumpPara.JumpWaitTime);
		}
		else
		{	
			ConfirmCounterJump = CONFIRMJUMP;
			switch(EepromPara.EepromParaS.JumpPara.JumpSEL)
			{
				case	MEANING_TIANBIAO_SEL_DIRECT:
					SystemDistanceAfterJump = TempDistance;
				break;
				case	MEANING_TIANBIAO_SEL_WAITTIME:
					if(WaitCounter <= 0)
					{
						WaitCounter=0;
						SystemDistanceAfterJump = TempDistance;
						RecordJump_WaitCounter++;
						RecordRealPerfToFRAM(RecordJump_WaitCounter,RECORD_JUMP_INDEX);
					}
					else if(MeterOutStatus != 1)
					{
						WaitCounter--;
				  		MeterOutStatus = 3;
						SystemDistanceAfterJump = MasterVariable.MasterVariablePV;
					}
				break;
				case	MEANING_TIANBIAO_SEL_JUMPSPEED:
					if(TempDistance < MasterVariable.MasterVariablePV)
					{
						SystemDistanceAfterJump = MasterVariable.MasterVariablePV-EepromPara.EepromParaS.JumpPara.JumpChangeSpeed;
						if(SystemDistanceAfterJump < TempDistance)
						{
							SystemDistanceAfterJump = TempDistance;
						}
					}
					else
					{
						SystemDistanceAfterJump = MasterVariable.MasterVariablePV+EepromPara.EepromParaS.JumpPara.JumpChangeSpeed;
						if(SystemDistanceAfterJump > TempDistance)
						{
							SystemDistanceAfterJump = TempDistance;
						}
					}
				break;
				case	MEANING_TIANBIAO_SEL_QUSHIGENZONG:
					if(TempDistance < MasterVariable.MasterVariablePV)
					{
						SystemDistanceAfterJump = MasterVariable.MasterVariablePV-Qushi_Speed;
						if(SystemDistanceAfterJump < TempDistance)
						{
							SystemDistanceAfterJump = TempDistance;
						}
					}
					else
					{
						SystemDistanceAfterJump = MasterVariable.MasterVariablePV+Qushi_Speed;
						if(SystemDistanceAfterJump > TempDistance)
						{
							SystemDistanceAfterJump = TempDistance;
						}
					}
				break;
				default:break;
			}
		}
	}
}

void	ClearNoSampleSignal(void)
{
	NoSampleSignal = 0;
}

void	SetOneSecondEvenValure(float TempEvenValure)
{
	OneSecondEvenValure = TempEvenValure;
}

uint16_t GetWaitCounter(void)
{
	return(WaitCounter);
}

float	GetMasterVariablePV(void)
{
	return(MasterVariable.MasterVariablePV);
}

float	GetPreOneSecondEvenValure(void)
{
	return(OneSecondEvenValure);
}
//失波、跳变处理(每一秒执行一次)
void	DistanceEchoProccess(void)
{
	NoSampleSignal++;
	if(NoSampleSignal >= NOSINGNAL_MAXTIME)
	{
		ClearLowSampleBuffer();
	}
	if(NoSampleSignal > NOSINGNAL_MAXTIME)
	{
		NoSampleSignal = NOSINGNAL_MAXTIME;
		WorkStatus |= 0x01;				//硬件故障标志位置1，表示硬件出现故障
                //HART模块需要配置故障标志位，这里设置故障标志bit
                HartDeviceStatusByteHandle(DeviceMalfunction,FALSE);
		ClearIWR1443Data();
	}
	else
	{
		WorkStatus &= 0xFE;				//硬件故障标志清零，表示硬件无故障
                //HART模块需要配置故障标志位,这里清除故障标志bit
                HartDeviceStatusByteHandle(DeviceMalfunction,TRUE);
	}
	
	if(GetBestEchoIndex() != BEST_WAVE_NOT_FIND)
	{
		WorkStatus &= 0xFD;			//把有无回波标志位清零
	}
	else
	{
		WorkStatus |= 0x02;			//把有无回波标志位置1 表示无回波
	}
	if(GetBlueToothStatus() == CONNECTED_BLTOOTH)
	{
		WorkStatus |= 0x04;
	}
	else
	{
		WorkStatus &= 0xFB;
	}
	OneSecondEvenValure = CalOneSecondPV();
//	WaveLostProcess(CalculateDistance(OneSecondEvenValure));
	JumpProccess(WaveLostProcess(CalculateDistance(OneSecondEvenValure)));						//JumpProccess
	SlowOut(CalculatePVEven());	
//	SystemPercentBeforeBiliBiaojiao = CalcPercent(SystemDistanceOutCurrent);
	CalcPercent(SystemDistanceOutCurrent);

//	CalcQushi_Speed();										//计算跟随速度
//	if(EepromPara.EepromParaS.LinearPara.RLineUseorUnuse == MEANING_XIANXING_BILI_YONG)
//	{
//		MasterVariable.MasterVariablePercent = CalcBiliBiaojiao(MasterVariable.MasterVariablePercent);
//	}
	RunActivationSystem();	
	CalcCurrent();
}

void	DelayStart(void)
{
	uint32_t TempTime;
	TempTime = HAL_GetTick();
	while((HAL_GetTick()-TempTime) < 10000)
	{
		Feed_IWDG();
	}
}

void	SetReBoot_Current(void)
{
//	CurrentFloat(REBOOTCURRENT);
//	lastpwm = 14000;
//	CurrentPWM = 14000;
//	EepromPara.EepromParaS.PWMCurrentPara.PWM4mA = PWM4MA_DEFAULT;
//	EepromPara.EepromParaS.PWMCurrentPara.PWM18mA = PWM18MA_DEFAULT;
	switch(EepromPara.EepromParaS.PWMCurrentPara.CurrentStartSEL)
	{
		case 0:
			CalcPercent(OneSecondEvenValure);
			CurrentFloat(MasterVariable.MasterVariableSetCurrent);
			DelayStart();
		break;
		case 1:
			CurrentFloat(4);
			DelayStart();
		break;
		case 2:
			CurrentFloat(20.5);
		break;
		case 3:
			CurrentFloat(22);
		break;
		default:
			CurrentFloat(12);
		break;
	}
	lastpwm = CurrentPWM;
	UpdateCurrentTime(CurrentPWM);
}

void	SetMeterOutStatus(uint8_t CurrentStatus)
{
	MeterOutStatus = CurrentStatus;
}

uint8_t	GetMeterOutStatus(void)
{
	return(MeterOutStatus);
}

float SetCurrentGet()
{
  return MasterVariable.MasterVariableSetCurrent;
}

