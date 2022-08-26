/* Includes ------------------------------------------------------------------*/
#include 	"i2c.h"
#include	"crc.h"
#include	"math.h"
#include	"Eeprom.h"
#include	"EchoHandle.h"

/*------------------------- External variables --------------------------------------------------------*/
extern	const	uint16_t Char_Para_Table[CHARPARANUMBERS][5];
extern	const	float Float_Para_Table[FLOATPARANUMBERS][5];
/*-----------------------------------local variables---------------------------------------------------*/
EepromParaU__TypeDef	EepromPara;
uint32_t ReadWriteEeprom_StartTime;
uint16_t StartAddressRom,SaveByteNumber;
uint16_t UserParaAddressIndex;
uint8_t AllUserPara_HandleMethod;
uint8_t ReadWriteEepromStatus;
uint8_t CharPara_FlagOfChange[CHARPARANUMBERS],CharParaAddress_FlagOfChange_Index;
uint8_t FloatPara_FlagOfChange[FLOATPARANUMBERS],FloatParaAddress_FlagOfChange_Index;
uint16_t ReadEepromErrorCount,WriteEepromErrorCount;
uint16_t ReadEepromTotalCount,WriteEepromTotalCount;
//uint16_t 
uint8_t EepromErrorFlag;
float PWM18mABias,PWM4mABias;
/*------------------------------------------------------------------------------------*/
void	SoftwareResetI2C2(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	HAL_I2C_DeInit(&hi2c2);
    	__HAL_I2C_DISABLE(&hi2c2);
	hi2c2.Instance->CR1 = 0;
    	__HAL_I2C_DISABLE(&hi2c2);
	hi2c2.Instance->CR2 = 0;
    	__HAL_I2C_DISABLE(&hi2c2);
	
	GPIO_InitStruct.Pin = EEROM_SCL_Pin|EEROM_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, EEROM_SCL_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, EEROM_SDA_Pin, GPIO_PIN_SET);

	hi2c2.State = HAL_I2C_STATE_RESET;
	hi2c2.Lock = HAL_UNLOCKED;
	MX_I2C2_Init();
}

void	EepromError_Handler(void)
{
	if((HAL_GetTick()-ReadWriteEeprom_StartTime) > READWRITEEEPROM_MAXTIME)
	{
		SoftwareResetI2C2();
	}
}

void	SetReadWriteEepromStatus(uint8_t TempStatus)
{
	ReadWriteEepromStatus = TempStatus;
	ReadWriteEeprom_StartTime = HAL_GetTick();
}

void	RecordReadEepromTimes(void)
{
	ReadEepromTotalCount++;
	while(ReadWriteEepromStatus != READEEPROM_END)
	{
		if((HAL_GetTick()-ReadWriteEeprom_StartTime) > READWRITEEEPROM_MAXTIME)
		{
			ReadEepromErrorCount++;
			SoftwareResetI2C2();
			ReadWriteEepromStatus = READEEPROM_END;
		}
	}
}

void	RecordWriteEepromTimes(void)
{
	WriteEepromTotalCount++;
	while(ReadWriteEepromStatus != WRITEEEPROM_END)
	{
		if((HAL_GetTick()-ReadWriteEeprom_StartTime) > READWRITEEEPROM_MAXTIME)
		{
//			WriteEepromErrorCount++;
			SoftwareResetI2C2();
			ReadWriteEepromStatus = WRITEEEPROM_END;
		}
	}
}

void	ReadEeprom(uint16_t Memory_Address,uint8_t *RxpData,uint8_t ByteNumber)
{
	/*##Start reading process ##############################################*/
	SetReadWriteEepromStatus(STARTREADWRITEEEPROM);
	if(HAL_I2C_Mem_Read_DMA(&hi2c2 , (uint16_t)EEPROM_ADDRESS, Memory_Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)RxpData, ByteNumber)!= HAL_OK)
	{
		/* Reading process Error */
		EepromError_Handler();
	}
	/* Wait for the end of the transfer */ 
	while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
	{
		EepromError_Handler();
//		if((HAL_GetTick()-ReadWriteEeprom_StartTime) > READWRITEEEPROM_MAXTIME)
//		{
//			HAL_I2C_DeInit(&hi2c2);
//			break;
//		}
	}
	RecordReadEepromTimes();
}

void	WriteEeprom(uint16_t Memory_Address,uint8_t *pData,uint8_t ByteNumber)
{
	uint8_t RxpData[50];
	uint8_t TempC;
	ENABLEWRITEEEPROM;
	SetReadWriteEepromStatus(STARTREADWRITEEEPROM);
	if(HAL_I2C_Mem_Write_DMA(&hi2c2, (uint16_t)EEPROM_ADDRESS, Memory_Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, ByteNumber)!= HAL_OK)
	{
		/* Writing process Error */
		EepromError_Handler();
	}

	while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
	{
		EepromError_Handler();
	} 

	/* Check if the EEPROM is ready for a new operation */  
	while (HAL_I2C_IsDeviceReady(&hi2c2, EEPROM_ADDRESS, 10, 300) == HAL_TIMEOUT)
	{
		EepromError_Handler();
	}

	/* Wait for the end of the transfer */  
	while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
	{
		EepromError_Handler();
//		HAL_I2C_DeInit(&hi2c2);
//		MX_I2C2_Init();
	}
	RecordWriteEepromTimes();
	ReadEeprom(Memory_Address,&RxpData[0],ByteNumber);
	for(TempC = 0; TempC < ByteNumber; TempC++)
	{//判断写入EEPROM的数据与读出来的数据是否一致
		if((*pData) != RxpData[TempC])
		{
			WriteEepromErrorCount++;
			TempC = ByteNumber;
		}
		pData++;
	}
	DISABLEWRITEEEPROM;
	/*记录最后一次更改参数的时间*/
	
}

void	ReadInitFlag(void)
{
	ReadEeprom(0,(uint8_t*)EepromPara.EepromParaC,2);
}

void	ReadMeterPara(void)
{
	uint16_t TempEepromAddress;
	uint16_t TempByteNumbers;
//	uint16_t TempCRC;
	TempByteNumbers = 200;
	TempEepromAddress = 0;
	do
	{
		if((TempEepromAddress+TempByteNumbers) > sizeof(EepromPara))
		{//判断剩余字节数小于200时处理
			TempByteNumbers = sizeof(EepromPara)-TempEepromAddress;
		}
		ReadEeprom(TempEepromAddress,(uint8_t*)(EepromPara.EepromParaC+TempEepromAddress),TempByteNumbers);
		TempEepromAddress = TempEepromAddress+TempByteNumbers;
	}while(TempEepromAddress < sizeof(EepromPara));
//	TempCRC = CRC_Accumulate((uint8_t*)(EepromPara.EepromParaC),sizeof(EepromPara)-1);
//	if(EepromPara.EepromParaS.EepromPara_CRC == 0xFFFF)
//	{
		
//	}
}

void	InitParaDefault(void)
{
	uint16_t TempPara_Address;
	uint16_t TempI;
	uint8_t TempC;
	union
	{
		float TempF;
		unsigned char TempChar[4];
	}TempFloatToChar;
	
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
	
	EepromPara.EepromParaS.LinearPara.LinePointsNum = 0;
	EepromPara.EepromParaS.LinearPara.ScalePointsNum = 0;
	for(TempC = 0; TempC < 20; TempC++)
	{
		EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][0] = 0;
		EepromPara.EepromParaS.LinearPara.LinearParaFB[TempC][1] = 0;
		EepromPara.EepromParaS.LinearPara.ScaleParaFB[TempC][0] = 0;
		EepromPara.EepromParaS.LinearPara.ScaleParaFB[TempC][1] = 0;
	}
	
	/*设定所有字符数参数默认值*/
	for(TempI = 0; TempI < CHARPARANUMBERS; TempI++)
	{
		TempPara_Address = Char_Para_Table[TempI][PARATABLE_INDEX_ADDRESS];
		EepromPara.EepromParaC[TempPara_Address] = Char_Para_Table[TempI][PARATABLE_INDEX_DEFAULT_V];
	}
	/*设定所有浮点数参数默认值*/
	for(TempI = 0; TempI < FLOATPARANUMBERS; TempI++)
	{
		TempPara_Address = (uint16_t)(Float_Para_Table[TempI][PARATABLE_INDEX_ADDRESS]);
		TempFloatToChar.TempF = Float_Para_Table[TempI][PARATABLE_INDEX_DEFAULT_V];
		for(TempC = 0; TempC < 4; TempC++)
			EepromPara.EepromParaC[TempPara_Address+TempC] = TempFloatToChar.TempChar[TempC];
	}
	
}

void	CheckPara(void)
{
	uint16_t TempPara_Address;
	uint16_t TempI;
	uint8_t TempC;
	union
	{
		float TempF;
		unsigned char TempChar[FLOAT_LENGTH];
	}TempFloatToChar;
	
	for(TempI = 0; TempI < FLOATPARANUMBERS; TempI++)
	{
		TempPara_Address = (uint16_t)(Float_Para_Table[TempI][PARATABLE_INDEX_ADDRESS]);
		for(TempC = 0; TempC < 4; TempC++)
		{
			TempFloatToChar.TempChar[TempC] = EepromPara.EepromParaC[TempPara_Address+TempC];
		}
		if((TempFloatToChar.TempF > Float_Para_Table[TempI][PARATABLE_INDEX_MAX_V])||
		   (TempFloatToChar.TempF < Float_Para_Table[TempI][PARATABLE_INDEX_MIN_V])||(isnan(TempFloatToChar.TempF) == 1))
		{
			TempFloatToChar.TempF = Float_Para_Table[TempI][PARATABLE_INDEX_DEFAULT_V];	
			for(TempC = 0; TempC < 4; TempC++)
				EepromPara.EepromParaC[TempPara_Address+TempC] = TempFloatToChar.TempChar[TempC];
		}
	}
	
	for(TempI = 0; TempI < CHARPARANUMBERS; TempI++)
	{
		TempPara_Address = Char_Para_Table[TempI][PARATABLE_INDEX_ADDRESS];
		if((EepromPara.EepromParaC[TempPara_Address] > Char_Para_Table[TempI][PARATABLE_INDEX_MAX_V])||
		   (EepromPara.EepromParaC[TempPara_Address] < Char_Para_Table[TempI][PARATABLE_INDEX_MIN_V])||(EepromPara.EepromParaC[TempPara_Address] == 0xFF))
		{
			EepromPara.EepromParaC[TempPara_Address] = Char_Para_Table[TempI][PARATABLE_INDEX_DEFAULT_V];
		}
	}
}

void	InitPara(void)
{
	uint16_t TempI;
	SaveByteNumber = 0;
	StartAddressRom = 0;
	AllUserPara_HandleMethod = USERPARA_NO_OPS;
	
	for(TempI = 0; TempI < CHARPARANUMBERS; TempI++)
	{
		CharPara_FlagOfChange[TempI] = DISABLE_WRITEEEPROM;
	}
	for(TempI = 0; TempI < FLOATPARANUMBERS; TempI++)
	{
		FloatPara_FlagOfChange[TempI] = DISABLE_WRITEEEPROM;
	}
	
	ReadMeterPara();
	if(EepromPara.EepromParaS.Eeprom_InitFlagI != NOT_FIRST_TIME)
	{
		EepromPara.EepromParaS.Eeprom_InitFlagI = NOT_FIRST_TIME;
		InitParaDefault();
		for(TempI = 0; TempI < sizeof(EepromPara.EepromParaS); TempI= TempI+EEPROM_PAGESIZE)
		{
			WriteEeprom(TempI,(uint8_t*)(EepromPara.EepromParaC+TempI),EEPROM_PAGESIZE);
//			HAL_Delay(6);
		}
	}
	CheckPara();
}

void	Write_Para_Multibyte(uint16_t ParaStartAddress,uint8_t ParaBytNumber)
{
	uint16_t TempC;	
	StartAddressRom = ParaStartAddress;
	TempC = StartAddressRom%EEPROM_PAGESIZE;
	if(TempC > (EEPROM_PAGESIZE-ParaBytNumber))
		SaveByteNumber = EEPROM_PAGESIZE-TempC;
	else
		SaveByteNumber = ParaBytNumber;
	WriteEeprom(StartAddressRom,(uint8_t*)(EepromPara.EepromParaC+StartAddressRom),SaveByteNumber);
	//HAL_I2C_Mem_Read(&hi2c2, (uint16_t)EEPROM_ADDRESS, StartAddressRom, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, SaveByteNumber);
	StartAddressRom = SaveByteNumber+StartAddressRom;
	SaveByteNumber = ParaBytNumber-SaveByteNumber;
}

void	SavePara_FloatType(uint16_t ParaAddress, float *ParaValueP)
{
//	uint16_t TempI;
	uint16_t TempPara_Address;
	uint8_t TempC;
	union
	{
		float TempF;
		unsigned char TempChar[4];
	}TempFloatToChar;
	
	if(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)
	{
		TempPara_Address = (uint16_t)(Float_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]);
		TempFloatToChar.TempF = *ParaValueP;
		if((TempFloatToChar.TempF <= Float_Para_Table[ParaAddress][PARATABLE_INDEX_MAX_V])&&
		   (TempFloatToChar.TempF >= Float_Para_Table[ParaAddress][PARATABLE_INDEX_MIN_V]))
		{
			FloatPara_FlagOfChange[ParaAddress] = ENABLE_WRITEEEPROM;
			for(TempC = 0; TempC < 4; TempC++)
			{
				EepromPara.EepromParaC[TempPara_Address+TempC] = TempFloatToChar.TempChar[TempC];
			}
		}
		/*------*/
		ParaConvert();
	}
}

void	SavePara_CharType(uint16_t ParaAddress, uint8_t ParaValue)
{
	if(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)
	{
		if((ParaValue <= Char_Para_Table[ParaAddress][PARATABLE_INDEX_MAX_V])&&
		  (ParaValue >= Char_Para_Table[ParaAddress][PARATABLE_INDEX_MIN_V]))
		{
			EepromPara.EepromParaC[Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]] = ParaValue;
			CharPara_FlagOfChange[ParaAddress] = ENABLE_WRITEEEPROM;
//			WriteEeprom(Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS],
//			    (uint8_t*)(EepromPara.EepromParaC+Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]),1);
		}
	}
}

void	SavePara_StringType(uint16_t ParaAddress, uint8_t *ParaValueP)
{
	uint16_t TempC;
	if(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)
	{
		for(TempC = 0; TempC < Char_Para_Table[ParaAddress][PARATABLE_INDEX_BYTES]; TempC++)
		{
			if((*ParaValueP >= Char_Para_Table[ParaAddress][PARATABLE_INDEX_MAX_V])||
			  (*ParaValueP <= Char_Para_Table[ParaAddress][PARATABLE_INDEX_MIN_V]))
			{
				EepromPara.EepromParaC[Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]+TempC] = Char_Para_Table[ParaAddress][PARATABLE_INDEX_DEFAULT_V];
			}
			else
			{
				EepromPara.EepromParaC[Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]+TempC] = *ParaValueP;
			}
			ParaValueP++;
		}

		Write_Para_Multibyte(Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS],Char_Para_Table[ParaAddress][PARATABLE_INDEX_BYTES]);
	}
}

void	SavePara_LineType(uint16_t ParaAddress, float *ParaValueP)
{
	uint16_t TempPara_Address;
	EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][0] = *ParaValueP;
	ParaValueP++;
	EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][1] = *ParaValueP;

	TempPara_Address = ((uint8_t volatile*)&EepromPara.EepromParaS.LinearPara.LinearParaFB-EepromPara.EepromParaC);
	Write_Para_Multibyte(TempPara_Address,8);
}

uint8_t *GetPara_Char(uint16_t ParaAddress)
{
	uint16_t TempPara_Address;
	TempPara_Address = (uint16_t)Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS];
	return(&EepromPara.EepromParaC[TempPara_Address]);
}

uint8_t *GetPara_Float(uint16_t ParaAddress)
{
	uint16_t TempPara_Address;
	TempPara_Address = (uint16_t)Float_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS];
	return(&EepromPara.EepromParaC[TempPara_Address]);
}

void	ReadParaL_LCD(uint16_t ParaAddress, float *ParaValueP)
{
	*ParaValueP = EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][0];
	ParaValueP++;
	*ParaValueP = EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][1];
}

void	Save_Eeprom_EnablePara_Protect(uint8_t *ParaValueP)
{
	uint8_t TempC;
	uint8_t TempReceiveData[13];
	
	for(TempC = 0; TempC < 13; TempC++)
	{
		TempReceiveData[TempC] = *ParaValueP++;
	}
	for(TempC = 0; TempC < 6; TempC++)
	{
		if(TempReceiveData[TempC] != ~TempReceiveData[TempC+6])
		{
			TempC = 0x5A;
			break;
		}
	}
	if(TempC != 0x5A)
	{
		EepromPara.EepromParaS.Eeprom_EnablePara_Protect = TempReceiveData[12]; /* 更新参数写保护功能标志 */
		WriteEeprom(Char_Para_Table[25][PARATABLE_INDEX_ADDRESS],
		    (uint8_t*)(EepromPara.EepromParaC+Char_Para_Table[25][PARATABLE_INDEX_ADDRESS]),BYTE_LENGTH);
	//HAL_I2C_Mem_Read(&hi2c2, (uint16_t)EEPROM_ADDRESS, StartAddressRom, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, SaveByteNumber);
	}
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
	uint16_t TempPara_Address;
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
		
		TempPara_Address = ((uint8_t volatile*)&EepromPara.EepromParaS.PWMCurrentPara.PWM18mA-EepromPara.EepromParaC);
		Write_Para_Multibyte(TempPara_Address,8);
//		HART_Variables.SettingCurrent = 18;
	}
}
void	SetAllUserPara_HandleMethod(uint8_t ParaValue)
{
	if(AllUserPara_HandleMethod == 0)
	{
		AllUserPara_HandleMethod = ParaValue;
		UserParaAddressIndex = 0;
	}
}

uint8_t	GetAllUserPara_HandleMethod(void)
{
	return(AllUserPara_HandleMethod);
}

void	SavePara_WriteEeprom()
{
	for(;CharParaAddress_FlagOfChange_Index < CHARPARANUMBERS; CharParaAddress_FlagOfChange_Index++)
	{
		if(CharPara_FlagOfChange[CharParaAddress_FlagOfChange_Index] == ENABLE_WRITEEEPROM)
		{
			CharPara_FlagOfChange[CharParaAddress_FlagOfChange_Index] = DISABLE_WRITEEEPROM;
			WriteEeprom(Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_ADDRESS],
			(uint8_t*)(EepromPara.EepromParaC+Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_ADDRESS]),
									Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_BYTES]);
			break;
		}
	}
	if(CharParaAddress_FlagOfChange_Index >= CHARPARANUMBERS)
	{
		CharParaAddress_FlagOfChange_Index = 0;
		for(;FloatParaAddress_FlagOfChange_Index < FLOATPARANUMBERS; FloatParaAddress_FlagOfChange_Index++)
		{
			if(FloatPara_FlagOfChange[FloatParaAddress_FlagOfChange_Index] == ENABLE_WRITEEEPROM)
			{
				FloatPara_FlagOfChange[FloatParaAddress_FlagOfChange_Index] = DISABLE_WRITEEEPROM;
				Write_Para_Multibyte((uint16_t)(Float_Para_Table[FloatParaAddress_FlagOfChange_Index][PARATABLE_INDEX_ADDRESS]),
									(uint8_t)(Float_Para_Table[FloatParaAddress_FlagOfChange_Index][PARATABLE_INDEX_BYTES]));
				break;
			}			
		}
		if(FloatParaAddress_FlagOfChange_Index >= FLOATPARANUMBERS)
		{
			FloatParaAddress_FlagOfChange_Index = 0;
		}
	}
}

void	SaveUserPara(void)
{
	if(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)
	{
		switch(AllUserPara_HandleMethod)
		{
			case	USERPARATOFACTORPARA:
				WriteEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),EEPROM_PAGESIZE);
				UserParaAddressIndex = UserParaAddressIndex+EEPROM_PAGESIZE;
				if((UserParaAddressIndex+EEPROM_PAGESIZE) > sizeof(EepromPara))
				{
					WriteEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),(sizeof(EepromPara)-UserParaAddressIndex));
					AllUserPara_HandleMethod = USERPARA_NO_OPS;
				}
			break;
			case	FACTORPARATOUSERPARA:
				ReadEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),EEPROM_PAGESIZE);
				UserParaAddressIndex = UserParaAddressIndex+EEPROM_PAGESIZE;
				if((UserParaAddressIndex+EEPROM_PAGESIZE) > sizeof(EepromPara))
				{
					ReadEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),(sizeof(EepromPara)-UserParaAddressIndex));
					AllUserPara_HandleMethod = USERPARA_NO_OPS;
				}
			break;
			case	DEFAULTPARATOUSERPARA:
				InitParaDefault();
				WriteEeprom(UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),EEPROM_PAGESIZE);
				UserParaAddressIndex = UserParaAddressIndex+EEPROM_PAGESIZE;
				if((UserParaAddressIndex+EEPROM_PAGESIZE) > sizeof(EepromPara))
				{
					WriteEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),(sizeof(EepromPara)-UserParaAddressIndex));
					AllUserPara_HandleMethod = USERPARA_NO_OPS;
				}
			break;
			default:
				AllUserPara_HandleMethod = USERPARA_NO_OPS;
				if(SaveByteNumber > 0)
				{
					WriteEeprom(StartAddressRom,(uint8_t*)(EepromPara.EepromParaC+StartAddressRom),SaveByteNumber);
					//HAL_I2C_Mem_Read(&hi2c2, (uint16_t)EEPROM_ADDRESS, StartAddressRom, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, SaveByteNumber);
					SaveByteNumber = 0;
				}
				else
				{
					SavePara_WriteEeprom();
				}
			break;
		}
	}
	else
	{
		AllUserPara_HandleMethod = USERPARA_NO_OPS;
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