/* Includes ------------------------------------------------------------------*/
#include 	"i2c.h"
#include	"Eeprom.h"
#include	"EchoHandle.h"
/*------------------------- External variables --------------------------------------------------------*/
//extern	I2C_HandleTypeDef hi2c2;
//-------------------------------------------------------------------------------------------------------------------------------------
EepromParaU__TypeDef	EepromPara;
uint16_t StartAddressRom,SaveByteNumber;
float PWM18mABias,PWM4mABias;
//--------------------------------------------------------------------------------------------------------------------------------------
const	uint16_t Char_Para_Table[23][4] = 
{
(&EepromPara.EepromParaS.EchoAdjudgePara.FakeWaveGain-EepromPara.EepromParaC),1,99,0,
(&EepromPara.EepromParaS.BasicPara.DampingTime-EepromPara.EepromParaC),1,99,0,
(&EepromPara.EepromParaS.BasicPara.DielectricConstant-EepromPara.EepromParaC),1,5,0,
(&EepromPara.EepromParaS.PWMCurrentPara.CurrentDirection-EepromPara.EepromParaC),1,1,0,
(&EepromPara.EepromParaS.SystemInformationP.Language-EepromPara.EepromParaC),1,2,0,
(&EepromPara.EepromParaS.BasicPara.PVUnit-EepromPara.EepromParaC),1,5,0,
(&EepromPara.EepromParaS.SystemInformationP.UserPassword[0]-EepromPara.EepromParaC),USERPASSWORD_LENGTH,250,0,
(&EepromPara.EepromParaS.ComPara.ComAddress-EepromPara.EepromParaC),1,99,0,
(&EepromPara.EepromParaS.ComPara.Method-EepromPara.EepromParaC),1,1,0,
(&EepromPara.EepromParaS.WaveLostPara.WaveLostProccessSEL-EepromPara.EepromParaC),1,3,0,
(&EepromPara.EepromParaS.JumpPara.JumpSEL-EepromPara.EepromParaC),1,4,0,
(&EepromPara.EepromParaS.LinearPara.DLineUseorUnuse-EepromPara.EepromParaC),1,1,0,
(&EepromPara.EepromParaS.LinearPara.RLineUseorUnuse-EepromPara.EepromParaC),1,1,0,
(&EepromPara.EepromParaS.PWMCurrentPara.CurrentAlarmSEL-EepromPara.EepromParaC),1,4,0,
(&EepromPara.EepromParaS.PWMCurrentPara.CurrentStartSEL-EepromPara.EepromParaC),1,4,0,
(&EepromPara.EepromParaS.EchoAdjudgePara.Threshold-EepromPara.EepromParaC),1,250,0,
(&EepromPara.EepromParaS.EchoAdjudgePara.EchoAdjudgeSEL-EepromPara.EepromParaC),1,4,0,
0,0,0,0,
(&EepromPara.EepromParaS.SystemInformationP.PDate[0]-EepromPara.EepromParaC),DATE_LENGTH,250,0,
(&EepromPara.EepromParaS.SystemInformationP.PSN[0]-EepromPara.EepromParaC),SENSOR_SN_LENGTH,250,0,
(&EepromPara.EepromParaS.SystemInformationP.PDescriptor[0]-EepromPara.EepromParaC),DESCRIPTOR_LENGTH,250,0,
(&EepromPara.EepromParaS.SystemInformationP.PTag[0]-EepromPara.EepromParaC),TAG_LENGTH,250,0,
(&EepromPara.EepromParaS.SystemInformationP.FinalAsmNo[0]-EepromPara.EepromParaC),FINAL_ASM_NO_LENGTH,250,0,
};

const	uint16_t Float_Para_Table[21][2] = 
{
((uint8_t volatile*)&EepromPara.EepromParaS.RangeValue.LowerRange-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.RangeValue.UpperRange-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.BasicPara.InputM_Speed-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.BasicPara.RadarLength-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.DistCoefficient.SystemZeroDist-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.DistCoefficient.SystemProportion-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.RangeLimit.UpperRange-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.RangeLimit.LowerRange-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.BasicPara.OutputM_Speed-EepromPara.EepromParaC),0,
0,0,
((uint8_t volatile*)&EepromPara.EepromParaS.PWMCurrentPara.PWM4mA-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.PWMCurrentPara.PWM18mA-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.MappingPara.MappingUpper-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.MappingPara.MappingLow-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.WaveLostPara.WaveLostSetPosition-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.JumpPara.JumpDist-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.JumpPara.JumpWaitTime-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.JumpPara.JumpChangeSpeed-EepromPara.EepromParaC),0,
((uint8_t volatile*)(&EepromPara.EepromParaS.EchoAdjudgePara.FrontEchoA)-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.EchoAdjudgePara.ConfirmEchoTime-EepromPara.EepromParaC),0,
((uint8_t volatile*)&EepromPara.EepromParaS.EchoAdjudgePara.MinDistinctiveness-EepromPara.EepromParaC),0,
};

void	WriteEeprom(uint16_t Memory_Address,uint8_t *pData,uint8_t ByteNumber)
{
	if(HAL_I2C_Mem_Write_DMA(&hi2c2, (uint16_t)EEPROM_ADDRESS, Memory_Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, ByteNumber)!= HAL_OK)
	{
		/* Writing process Error */
		_Error_Handler(__FILE__, __LINE__);
	}

    /* Wait for the end of the transfer */
    /*  Before starting a new communication transfer, you need to check the current
      state of the peripheral; if it抯 busy you need to wait for the end of current transfer before starting a new one.
      For simplicity reasons, this example is just waiting till the end of the transfer, 
	but application may perform other tasks while transfer operation is ongoing. */
	while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
	{
	} 

	/* Check if the EEPROM is ready for a new operation */  
	while (HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_ADDRESS, 10, 300) == HAL_TIMEOUT);

	/* Wait for the end of the transfer */  
	while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
	{
	}
}

void	ReadEeprom(uint16_t Memory_Address,uint8_t *RxpData,uint8_t ByteNumber)
{
	/*##Start reading process ##############################################*/
	if(HAL_I2C_Mem_Read_DMA(&hi2c2 , (uint16_t)EEPROM_ADDRESS, Memory_Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)RxpData, ByteNumber)!= HAL_OK)
	{
		/* Reading process Error */
		_Error_Handler(__FILE__, __LINE__);
	}
	/* Wait for the end of the transfer */  
	while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
	{
	}
}

void	ReadInitFlag(void)
{
	ReadEeprom(0,(uint8_t*)EepromPara.EepromParaC,2);
}

void	ReadMeterPara(void)
{
	ReadEeprom(0,(uint8_t*)EepromPara.EepromParaC,250);
}

void	InitParaDefault(void)
{
	uint8_t TempC;
	
	EepromPara.EepromParaS.SystemInformationP.PDate[0] = '2';
	EepromPara.EepromParaS.SystemInformationP.PDate[1] = '0';
	EepromPara.EepromParaS.SystemInformationP.PDate[2] = '1';
	EepromPara.EepromParaS.SystemInformationP.PDate[3] = '7';
	EepromPara.EepromParaS.SystemInformationP.PDate[4] = '0';
	EepromPara.EepromParaS.SystemInformationP.PDate[5] = '8';
	EepromPara.EepromParaS.SystemInformationP.PDate[6] = '0';
	EepromPara.EepromParaS.SystemInformationP.PDate[7] = '1';

	for(TempC = 0; TempC < SENSOR_SN_LENGTH; TempC++)
		EepromPara.EepromParaS.SystemInformationP.PSN[TempC] = 0;
	for(TempC = 0; TempC < DESCRIPTOR_LENGTH; TempC++)
		EepromPara.EepromParaS.SystemInformationP.PDescriptor[TempC] = 0;

	EepromPara.EepromParaS.SystemInformationP.PTag[0] = 'S';
	EepromPara.EepromParaS.SystemInformationP.PTag[1] = 'E';
	EepromPara.EepromParaS.SystemInformationP.PTag[2] = 'N';
	EepromPara.EepromParaS.SystemInformationP.PTag[3] = 'S';
	EepromPara.EepromParaS.SystemInformationP.PTag[4] = 'O';
	EepromPara.EepromParaS.SystemInformationP.PTag[5] = 'R';
	
	EepromPara.EepromParaS.BasicPara.DampingTime = DAMPINGTIME_DEFAULT;
	EepromPara.EepromParaS.BasicPara.PVUnit = PV_UNIT_DEFAULT;
	EepromPara.EepromParaS.BasicPara.Material = 1;
	EepromPara.EepromParaS.BasicPara.DielectricConstant = 3;
	EepromPara.EepromParaS.BasicPara.OutputM_Speed = 0.5;
	EepromPara.EepromParaS.BasicPara.InputM_Speed = 0.5;
	EepromPara.EepromParaS.BasicPara.RadarLength = 0;

	EepromPara.EepromParaS.ComPara.Method = 0;
	EepromPara.EepromParaS.ComPara.ComAddress = 1;
	
	EepromPara.EepromParaS.DistCoefficient.SystemZeroDist = 0;
	EepromPara.EepromParaS.DistCoefficient.SystemProportion = 1;

	EepromPara.EepromParaS.RangeValue.UpperRange = UPPERRANGE_DEFAULT;
	EepromPara.EepromParaS.RangeValue.LowerRange = LOWERRANGE_DEFAULT;
	EepromPara.EepromParaS.RangeLimit.UpperRange = UPPERRANGELIMIT_DEFAULT;
	EepromPara.EepromParaS.RangeLimit.LowerRange = LOWERRANGELIMIT_DEFAULT;
	
	EepromPara.EepromParaS.PWMCurrentPara.PWM18mA = PWM18MA_DEFAULT;
	EepromPara.EepromParaS.PWMCurrentPara.PWM4mA = PWM4MA_DEFAULT;
	EepromPara.EepromParaS.PWMCurrentPara.CurrentDirection = 0;
	EepromPara.EepromParaS.PWMCurrentPara.CurrentAlarmSEL = 2;
	EepromPara.EepromParaS.PWMCurrentPara.CurrentStartSEL = 2;
	
	EepromPara.EepromParaS.WaveLostPara.WaveLostProccessSEL = WAVELOST_SEL_REMAIN;
	EepromPara.EepromParaS.WaveLostPara.WaveLostSetPosition = 0;

	EepromPara.EepromParaS.JumpPara.JumpSEL = JUMP_SEL_DIRECT;
	EepromPara.EepromParaS.JumpPara.JumpDist = 0.5;
	EepromPara.EepromParaS.JumpPara.JumpWaitTime = 60;
	EepromPara.EepromParaS.JumpPara.JumpChangeSpeed = 0.2;
	
	EepromPara.EepromParaS.EchoAdjudgePara.EchoAdjudgeSEL = ECHOADJUDGE_SEL_MAX;
	EepromPara.EepromParaS.EchoAdjudgePara.ConfirmEchoTime = 10;
	EepromPara.EepromParaS.EchoAdjudgePara.MinDistinctiveness = 0.2;
	EepromPara.EepromParaS.EchoAdjudgePara.FrontEchoA = 0.2;
	EepromPara.EepromParaS.EchoAdjudgePara.FakeWaveGain = 15;
	EepromPara.EepromParaS.EchoAdjudgePara.Threshold = 20;
	
	EepromPara.EepromParaS.LinearPara.DLineUseorUnuse = 0;
	EepromPara.EepromParaS.LinearPara.RLineUseorUnuse = 0;
	EepromPara.EepromParaS.LinearPara.LinePointsNum = 0;
	EepromPara.EepromParaS.LinearPara.ScalePointsNum = 0;
	for(TempC = 0; TempC < 20; TempC++)
	{
		EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][0] = 0;
		EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][1] = 0;
		EepromPara.EepromParaS.LinearPara.ScaleParaFB[TempC][0] = 0;
		EepromPara.EepromParaS.LinearPara.ScaleParaFB[TempC][1] = 0;
	}
}

void	InitPara(void)
{
	uint16_t TempI;
	SaveByteNumber = 0;
	StartAddressRom = 0;
	ReadMeterPara();
	if(EepromPara.EepromParaS.Eeprom_InitFlagI != NOT_FIRST_TIME)
	{
		EepromPara.EepromParaS.Eeprom_InitFlagI = NOT_FIRST_TIME;
		InitParaDefault();
		for(TempI = 0; TempI < sizeof(EepromPara.EepromParaS); TempI= TempI+32)
		{
			WriteEeprom(TempI,(uint8_t*)(EepromPara.EepromParaC+TempI),32);
			HAL_Delay(6);
		}
	}
}

void	Write_Para_Multibyte(uint8_t ParaBytNumber)
{
	uint16_t TempC;	
	//-------------起始地址在Rom的一页里第几个字节开始---------------
	TempC = StartAddressRom%EEPROM_PAGESIZE;
	if(TempC > (EEPROM_PAGESIZE-ParaBytNumber))
		SaveByteNumber = EEPROM_PAGESIZE-TempC;//在同一页里存不完，还得下一次存到下一页
	else
		SaveByteNumber = ParaBytNumber;
	WriteEeprom(StartAddressRom,(uint8_t*)(EepromPara.EepromParaC+StartAddressRom),SaveByteNumber);
	StartAddressRom = SaveByteNumber+StartAddressRom;
	SaveByteNumber = ParaBytNumber-SaveByteNumber;
}

void	SavePara_FloatType(uint16_t ParaAddress, float *ParaValueP, uint8_t ParaNumber)
{
	uint16_t TempI;
	uint8_t TempC;
	union
	{
		float TempF;
		unsigned char TempChar[4];
	}TempFloatToChar;
	
	for(TempC = 0; TempC < ParaNumber; TempC++)
	{
		TempFloatToChar.TempF = *ParaValueP;
		for(TempI = 0; TempI < 4; TempI++)
			EepromPara.EepromParaC[Float_Para_Table[ParaAddress-34+TempC][0]+TempI] = TempFloatToChar.TempChar[TempI];
		ParaValueP++;
	}
	StartAddressRom = Float_Para_Table[ParaAddress-34][0];
	Write_Para_Multibyte(4*ParaNumber);
	//------
	ParaConvert();
}

void	SavePara_CharType(uint16_t ParaAddress, uint8_t ParaValue)
{
	if(ParaValue > Char_Para_Table[ParaAddress-10][2])
		ParaValue = Char_Para_Table[ParaAddress-10][2];
	EepromPara.EepromParaC[Char_Para_Table[ParaAddress-10][0]] = ParaValue;
	WriteEeprom(Char_Para_Table[ParaAddress-10][0],(uint8_t*)(EepromPara.EepromParaC+Char_Para_Table[ParaAddress-10][0]),1);
}

void	SavePara_StringType(uint16_t ParaAddress, uint8_t *ParaValueP)
{
	uint16_t TempC;
	for(TempC = 0; TempC < Char_Para_Table[ParaAddress-10][1]; TempC++)
		EepromPara.EepromParaC[Char_Para_Table[ParaAddress-10][0]+TempC] = *ParaValueP++;

	StartAddressRom = Char_Para_Table[ParaAddress-10][0];
	Write_Para_Multibyte(Char_Para_Table[ParaAddress-10][1]);
}

void	SavePara_LineType(uint16_t ParaAddress, float *ParaValueP)
{
	EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][0] = *ParaValueP;
	ParaValueP++;
	EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][1] = *ParaValueP;

	StartAddressRom = ((uint8_t volatile*)&EepromPara.EepromParaS.LinearPara.LinearParaFB-EepromPara.EepromParaC);
	Write_Para_Multibyte(8);
}

void	SavePara_NextPage_Rom()
{
	if(SaveByteNumber > 0)
	{
		WriteEeprom(StartAddressRom,(uint8_t*)(EepromPara.EepromParaC+StartAddressRom),SaveByteNumber);
		SaveByteNumber = 0;
	}
}

void	ReadParaF_LCD(uint16_t ParaAddress, uint8_t *ParaValueP, uint8_t ParaNumber)
{
	uint8_t TempC;
	for(TempC = 0; TempC < ParaNumber; TempC++)
	{
		*ParaValueP = EepromPara.EepromParaC[Float_Para_Table[ParaAddress-34][0]+TempC];
		ParaValueP++;
	}
}

void	ReadParaF_Hart(uint16_t ParaAddress, uint8_t *ParaValueP)
{
	uint8_t TempC;
	for(TempC = 3; TempC > 0; TempC--)
	{
		*ParaValueP = EepromPara.EepromParaC[Float_Para_Table[ParaAddress-34][0]+TempC];
		ParaValueP++;
	}
	*ParaValueP = EepromPara.EepromParaC[Float_Para_Table[ParaAddress-34][0]];
}

void	ReadParaC_LCD(uint16_t ParaAddress, uint8_t *ParaValueP, uint8_t ParaNumber)
{
	uint8_t TempC;
	for(TempC = 0; TempC < ParaNumber; TempC++)
	{
		if(ParaAddress == 10)
			*ParaValueP = EepromPara.EepromParaC[Char_Para_Table[ParaAddress+TempC-10][0]];
		else
			*ParaValueP = EepromPara.EepromParaC[Char_Para_Table[ParaAddress-10][0]+TempC];
		ParaValueP++;
	}
}

void	ReadParaL_LCD(uint16_t ParaAddress, float *ParaValueP)
{
	*ParaValueP = EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][0];
	ParaValueP++;
	*ParaValueP = EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][1];
}

void	Set_PWM18mABias_Value(float ParaValue)
{
	PWM18mABias = ParaValue;
}

void	Set_PWM4mABias_Value(float ParaValue)
{
	PWM4mABias = ParaValue;
}

void	ModfyCurrent(void)
{
	float paraa,parab;
	if(PWM18mABias > PWM4mABias)
	{
		paraa = EepromPara.EepromParaS.PWMCurrentPara.PWM4mA-EepromPara.EepromParaS.PWMCurrentPara.PWM18mA;
		paraa=paraa/(PWM18mABias-PWM4mABias);

		parab = (18-PWM18mABias)*paraa;
		if((parab > EepromPara.EepromParaS.PWMCurrentPara.PWM18mA)&&(parab > 0))
			EepromPara.EepromParaS.PWMCurrentPara.PWM18mA = 0;
		else
			EepromPara.EepromParaS.PWMCurrentPara.PWM18mA = EepromPara.EepromParaS.PWMCurrentPara.PWM18mA-parab;
		EepromPara.EepromParaS.PWMCurrentPara.PWM4mA = EepromPara.EepromParaS.PWMCurrentPara.PWM4mA+(PWM4mABias-4)*paraa;

		if(EepromPara.EepromParaS.PWMCurrentPara.PWM4mA < EepromPara.EepromParaS.PWMCurrentPara.PWM18mA)
		{
			EepromPara.EepromParaS.PWMCurrentPara.PWM18mA = PWM18MA_DEFAULT;
			EepromPara.EepromParaS.PWMCurrentPara.PWM4mA = PWM4MA_DEFAULT;
		}
		
		StartAddressRom = ((uint8_t volatile*)&EepromPara.EepromParaS.PWMCurrentPara.PWM18mA-EepromPara.EepromParaC);
		Write_Para_Multibyte(8);
//		HART_Variables.SettingCurrent = 18;
	}
}

uint8_t	GetComMethod(void)
{
	return(EepromPara.EepromParaS.ComPara.Method);
}

uint8_t	GetComAddress(void)
{
	return(EepromPara.EepromParaS.ComPara.ComAddress);
}