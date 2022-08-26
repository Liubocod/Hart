/* Includes ------------------------------------------------------------------*/
#include 	"i2c.h"
#include	"crc.h"
#include	"usart.h"
#include	"math.h"
#include	"Eeprom.h"
#include	"EchoHandle.h"
#include	"perf_management.h"
#include	"BlueTooth_Com.h"
#include	"LCD485.h"

#include 	"HartHandle.h"

/*------------------------- External variables --------------------------------------------------------*/
extern	const	uint16_t Char_Para_Table[CHARPARANUMBERS][5];
extern	const	float Float_Para_Table[FLOATPARANUMBERS][5];
extern	uint16_t FakeWave[SAMPLE_DATA_LENGTH];
/*-----------------------------------local variables---------------------------------------------------*/
const	float UnitPara_Ratio[5] = {3.2808,1,39.37,100,1000};
uint8_t PartBuffer_Out[4];

RecordModifyParaU__TypeDef RecordModifyPara;
EepromParaU__TypeDef	EepromPara;
uint32_t ReadWriteEeprom_StartTime,WriteEEpromInterval;
uint16_t StartAddressRom,SaveByteNumber;
uint16_t UserParaAddressIndex;
uint8_t AllUserPara_HandleMethod;
uint8_t ReadWriteEepromStatus;
uint8_t CharPara_FlagOfChange[CHARPARANUMBERS],CharParaAddress_FlagOfChange_Index;
uint8_t FloatPara_FlagOfChange[FLOATPARANUMBERS],FloatParaAddress_FlagOfChange_Index;
uint8_t FakeInfo_FlagOfChange;
uint8_t FakeInfo_Index,ReadFake_Status;
uint8_t CRCParaAddress_FlagOfChange;
uint16_t ReadEepromErrorCount,WriteEepromErrorCount;
uint16_t ReadEepromTotalCount,WriteEepromTotalCount;
uint16_t CheckRamPara_Time;
uint8_t EepromErrorFlag;
uint8_t SaveModifyParaInfoFlag = DISABLE;
uint8_t ModifyParaInfo_Count = 0;
float UserMaxLowerRange;
float PWM18mABias,PWM4mABias;
float UnitConv_Ratio;
/*-------------------------------------------------------------*/
/*---------液晶上显示的产品型号（ASCII码）转换成蓝牙定义的产品型号（十六进制）---------*/

void	Update_LPUART1_UART_Para(void)
{
	hlpuart1.Instance = LPUART1;
	HAL_UART_DeInit(&hlpuart1);
	hlpuart1.gState = HAL_UART_STATE_RESET;
	switch(EepromPara.EepromParaS.ComPara.BaudRate)
	{
		case 0:
			hlpuart1.Init.BaudRate = 1200;
		break;
		case 1:
			hlpuart1.Init.BaudRate = 2400;
		break;
		case 2:
			hlpuart1.Init.BaudRate = 4800;
		break;
		case 3:
			hlpuart1.Init.BaudRate = 9600;
		break;
		case 4:
			hlpuart1.Init.BaudRate = 19200;
		break;
		case 5:
			hlpuart1.Init.BaudRate = 38400;
		break;
		case 6:
			hlpuart1.Init.BaudRate = 57600;
		break;
		case 7:
			hlpuart1.Init.BaudRate = 115200;
		break;
		default:
			hlpuart1.Init.BaudRate = 1200;
		break;
	}
	
	switch(EepromPara.EepromParaS.ComPara.WordLength)
	{
		case 0:
			hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
		break;
		case 1:
			hlpuart1.Init.WordLength = UART_WORDLENGTH_7B;
		break;
		case 2:
			hlpuart1.Init.WordLength = UART_WORDLENGTH_9B;
		break;
		default:
			hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
		break;
	}

	switch(EepromPara.EepromParaS.ComPara.Parity)
	{
		case 0:
			hlpuart1.Init.Parity = UART_PARITY_NONE;
		break;
		case 1:
			hlpuart1.Init.Parity = UART_PARITY_EVEN;
		break;
		case 2:
			hlpuart1.Init.Parity = UART_PARITY_ODD;
		break;
		default:
			hlpuart1.Init.Parity = UART_PARITY_NONE;
		break;
	}
	
	switch(EepromPara.EepromParaS.ComPara.StopBits)
	{
		case 0:
			hlpuart1.Init.StopBits = UART_STOPBITS_1;
		break;
		case 1:
			hlpuart1.Init.StopBits = UART_STOPBITS_1_5;
		break;
		case 2:
			hlpuart1.Init.StopBits = UART_STOPBITS_2;
		break;
		default:
			hlpuart1.Init.StopBits = UART_STOPBITS_1;
		break;
	}
	hlpuart1.Init.Mode = UART_MODE_TX_RX;
	hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_RS485Ex_Init(&hlpuart1, UART_DE_POLARITY_HIGH, 30, 30) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	EnableLCD485Receive();
}

void	StringToInt_ProductType(void)
{
	uint8_t TempString[9];
	uint8_t TempC;
	SetProductType_Int(0);
	UserMaxLowerRange = Float_Para_Table[0][PARATABLE_INDEX_MAX_V];
	for(TempC = 0; TempC < 9; TempC++)
	{
		TempString[TempC] = *(GetPara_Char(20)+TempC);
	}
	if((EepromPara.EepromParaS.ExpandPara.CharPara[0] == 0x05)||(EepromPara.EepromParaS.ExpandPara.CharPara[0] == 0x02))
	{//根据厂家解析产品型号
		if((TempString[0] == 'C')&&(TempString[1] == 'P')&&(TempString[2] == 'R')&&(TempString[3] == '6')&&(TempString[5] == '0')&&(TempString[6] == '0'))
		{
			if(TempString[4] != 0x34)
			{
				SetProductType_Int(0x0010+TempString[4]-0x30);
			}
			else
			{
				if(TempString[8] == 'D')
				{
					SetProductType_Int(0x0021);
					UserMaxLowerRange = USERMAXLOWERRANGE_D;
				}
				else if(TempString[8] == 'R')
				{
					SetProductType_Int(0x0022);					
					UserMaxLowerRange = USERMAXLOWERRANGE_R;
				}
				else if(TempString[8] == 'U')
				{
					SetProductType_Int(0x0023);					
					UserMaxLowerRange = USERMAXLOWERRANGE_U;
				}
				else if(TempString[8] == 'Z')
				{
					SetProductType_Int(0x0024);					
				}
				else
				{
					SetProductType_Int(0x0014);					
				}
			}
		}

		if((TempString[0] == 'C')&&(TempString[1] == 'G')&&(TempString[2] == 'R')&&(TempString[3] == '8')&&(TempString[5] == '0')&&(TempString[6] == '0'))
		{
			SetProductType_Int(0x0040+TempString[4]-0x30);
		}
	}
	if(EepromPara.EepromParaS.RangeValue.LowerRange > UserMaxLowerRange)
	{
		EepromPara.EepromParaS.RangeValue.LowerRange = UserMaxLowerRange;
	}
}

/*---------液晶上显示的产品系列号（ASCII码）转换成蓝牙定义的产品序列号（十六进制）---------*/
void	StringToInt_ProductSL(void)
{
	uint16_t TempInt = 1;
	uint16_t TempProductSL = 0;
	uint8_t TempC;
	for(TempC = 9; TempC > 4; TempC--)
	{
		if(EepromPara.EepromParaS.SystemInformationP.PSN[TempC] >= 0x30)
		{
			TempProductSL = TempProductSL+(EepromPara.EepromParaS.SystemInformationP.PSN[TempC]-0x30)*TempInt;
		}
		else
		{
		}
		TempInt = TempInt*10;
	}
	SetProductSL_Int(TempProductSL);
}

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

void	WriteEEprom_DelayTime(void)
{
	uint32_t WriteEeprom_StartTime;
	WriteEeprom_StartTime = HAL_GetTick();
	while((HAL_GetTick()-WriteEeprom_StartTime) < WRITEEEPROM_DELAY)
	{
	;
	}
}

void	RecordReadEepromTimes(void)
{
//	ReadEepromTotalCount++;
	RecordPerfToFRAM(READEEPROMTOTALTIMES);
	while(ReadWriteEepromStatus != READEEPROM_END)
	{
		if((HAL_GetTick()-ReadWriteEeprom_StartTime) > READWRITEEEPROM_MAXTIME)
		{
//			ReadEepromErrorCount++;
			RecordPerfToFRAM(READEEPROMERRORTIMES);
			SoftwareResetI2C2();
			ReadWriteEepromStatus = READEEPROM_END;
		}
	}
}

void	RecordWriteEepromTimes(void)
{
//	WriteEepromTotalCount++;
	RecordPerfToFRAM(WRITEEEPROMTOTALTIMES);
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

void	RecordPWMCurrentPara(void)
{
	RecordRealPerfToFRAM(EepromPara.EepromParaS.PWMCurrentPara.PWM18mA,PWM18MA_P);
	RecordRealPerfToFRAM(EepromPara.EepromParaS.PWMCurrentPara.PWM4mA,PWM4MA_P);
//	RecordRealPerfToFRAM(ModifyParaInfo_Count,MODIFYPARINFO_P);		
}

void	ReadEeprom(uint16_t Memory_Address,uint8_t *RxpData,uint16_t ByteNumber)
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

void	WriteEeprom(uint16_t Memory_Address,uint8_t *pData,uint16_t ByteNumber)
{
	uint8_t RxpData[50];
	uint8_t TempC;
	uint8_t OnePage_ByteNumber;
	uint8_t TempByteNumber;
	ENABLEWRITEEEPROM;
	HAL_Delay(WRITEEEPROM_DELAY);
	SetReadWriteEepromStatus(STARTREADWRITEEEPROM);
	
	OnePage_ByteNumber = EEPROM_PAGESIZE-Memory_Address%EEPROM_PAGESIZE;
	if(OnePage_ByteNumber > 0)
	{
		if(OnePage_ByteNumber >= ByteNumber)
		{
			if(HAL_I2C_Mem_Write_DMA(&hi2c2, (uint16_t)EEPROM_ADDRESS, Memory_Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, ByteNumber)!= HAL_OK)
			{
				/* Writing process Error */
				EepromError_Handler();
			}
		}
		else
		{
			if(HAL_I2C_Mem_Write_DMA(&hi2c2, (uint16_t)EEPROM_ADDRESS, Memory_Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, OnePage_ByteNumber)!= HAL_OK)
			{
				/* Writing process Error */
				EepromError_Handler();
			}
			TempByteNumber = OnePage_ByteNumber+EEPROM_PAGESIZE;
			{
				HAL_Delay(WRITEEEPROM_DELAY);
				if(TempByteNumber > ByteNumber)
				{
					if(HAL_I2C_Mem_Write_DMA(&hi2c2, (uint16_t)EEPROM_ADDRESS, (Memory_Address+TempByteNumber-EEPROM_PAGESIZE), I2C_MEMADD_SIZE_16BIT, (uint8_t*)(pData+TempByteNumber-EEPROM_PAGESIZE), (ByteNumber+EEPROM_PAGESIZE-TempByteNumber))!= HAL_OK)
					{
						/* Writing process Error */
						EepromError_Handler();
					}
				}
				else
				{
					if(HAL_I2C_Mem_Write_DMA(&hi2c2, (uint16_t)EEPROM_ADDRESS, (Memory_Address+TempByteNumber-EEPROM_PAGESIZE), I2C_MEMADD_SIZE_16BIT, (uint8_t*)(pData+TempByteNumber-EEPROM_PAGESIZE), EEPROM_PAGESIZE)!= HAL_OK)
					{
						/* Writing process Error */
						EepromError_Handler();
					}
				}
				TempByteNumber = TempByteNumber+EEPROM_PAGESIZE;
			}while(TempByteNumber < ByteNumber);
		}
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
	if(ByteNumber < 20)
	{
		RecordWriteEepromTimes();
		ReadEeprom(Memory_Address,&RxpData[0],ByteNumber);
		for(TempC = 0; TempC < ByteNumber; TempC++)
		{//判断写入EEPROM的数据与读出来的数据是否一致
			if((*pData) != RxpData[TempC])
			{
//				WriteEepromErrorCount++;
				RecordPerfToFRAM(WRITEEEPROMERRORTIMES);
				TempC = ByteNumber;
			}
			pData++;
		}
//		RecordLastModfyParaTime();
	}
	DISABLEWRITEEEPROM;
	/*记录最后一次更改参数的时间*/	
}

void	CalculateParaCRC(void)
{
	EepromPara.EepromParaS.EepromPara_CRC = HAL_CRC_Calculate(&hcrc,(uint32_t *)(EepromPara.EepromParaC),sizeof(EepromPara)-4);
	CRCParaAddress_FlagOfChange = ENABLE_WRITEEEPROM;
}

void	ReadInitFlag(void)
{
	ReadEeprom(0,(uint8_t*)EepromPara.EepromParaC,2);
}

void	InitSystemInformation(void)
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
	
	EepromPara.EepromParaS.SystemInformationP.FinalAsmNo[0] = 0x30;
	EepromPara.EepromParaS.SystemInformationP.FinalAsmNo[1] = 0x30;
	EepromPara.EepromParaS.SystemInformationP.FinalAsmNo[2] = 0x30;
	
	EepromPara.EepromParaS.SystemInformationP.UserPassword[0] = 0x30;
	EepromPara.EepromParaS.SystemInformationP.UserPassword[1] = 0x30;
	EepromPara.EepromParaS.SystemInformationP.UserPassword[2] = 0x30;
	EepromPara.EepromParaS.SystemInformationP.UserPassword[3] = 0x30;
	EepromPara.EepromParaS.SystemInformationP.UserPassword[4] = 0x30;
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
		if((TempI != FACTORY_ID_ADDRESS)&&((TempI < 18)||(TempI > 22)))
		{
			TempPara_Address = (uint16_t)(Char_Para_Table[TempI][PARATABLE_INDEX_ADDRESS]);
			EepromPara.EepromParaC[TempPara_Address] = Char_Para_Table[TempI][PARATABLE_INDEX_DEFAULT_V];
		}
	}
	/*设定所有浮点数参数默认值*/
	for(TempI = 0; TempI < FLOATPARANUMBERS; TempI++)
	{
		TempPara_Address = (uint16_t)(Float_Para_Table[TempI][PARATABLE_INDEX_ADDRESS]);
		TempFloatToChar.TempF = Float_Para_Table[TempI][PARATABLE_INDEX_DEFAULT_V];
		for(TempC = 0; TempC < 4; TempC++)
			EepromPara.EepromParaC[TempPara_Address+TempC] = TempFloatToChar.TempChar[TempC];
	}
//		EepromPara.EepromParaS.EepromPara_CRC = HAL_CRC_Calculate(&hcrc,(uint32_t *)(EepromPara.EepromParaC),sizeof(EepromPara)-4);
}

void	ReadMeterPara(void)
{
	uint16_t TempEepromAddress;
//	uint16_t TempByteNumbers;
//	uint16_t TempCRC;
//	TempByteNumbers = 200;
	TempEepromAddress = 0;
//	do
//	{
//		if((TempEepromAddress+TempByteNumbers) > sizeof(EepromPara))
//		{//判断剩余字节数小于200时处理
//			TempByteNumbers = sizeof(EepromPara)-TempEepromAddress;
//		}
		ReadEeprom(TempEepromAddress,(uint8_t*)(EepromPara.EepromParaC+TempEepromAddress),sizeof(EepromPara));
//		TempEepromAddress = TempEepromAddress+TempByteNumbers;
//	}while(TempEepromAddress < sizeof(EepromPara));
//	TempCRC = HAL_CRC_Calculate(&hcrc,(uint32_t *)(EepromPara.EepromParaC),sizeof(EepromPara)-4);
//	if((EepromPara.EepromParaS.EepromPara_CRC == 0xFFFF)||(EepromPara.EepromParaS.EepromPara_CRC != TempCRC))
//	{
//		InitParaDefault();
//	}
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
//			RecordPerfToFRAM(SETDEFAULTPARATIMES);
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
//			RecordPerfToFRAM(SETDEFAULTPARATIMES);
			CharPara_FlagOfChange[TempI] = ENABLE_WRITEEEPROM;
			EepromPara.EepromParaC[TempPara_Address] = Char_Para_Table[TempI][PARATABLE_INDEX_DEFAULT_V];
		}
	}
	
	for(TempI = 0; TempI < DATE_LENGTH; TempI++)
	{
		TempPara_Address = Char_Para_Table[18][PARATABLE_INDEX_ADDRESS];
		if((EepromPara.EepromParaS.SystemInformationP.PDate[TempI] > Char_Para_Table[18][PARATABLE_INDEX_MAX_V])||
		   (EepromPara.EepromParaS.SystemInformationP.PDate[TempI] < Char_Para_Table[18][PARATABLE_INDEX_MIN_V])||(EepromPara.EepromParaS.SystemInformationP.PDate[TempI] == 0xFF))
		{
			EepromPara.EepromParaS.SystemInformationP.PDate[TempI] = Char_Para_Table[18][PARATABLE_INDEX_DEFAULT_V];
		}
	}
	
	for(TempI = 0; TempI < SENSOR_SN_LENGTH; TempI++)
	{
		TempPara_Address = Char_Para_Table[19][PARATABLE_INDEX_ADDRESS];
		if((EepromPara.EepromParaS.SystemInformationP.PSN[TempI] > Char_Para_Table[19][PARATABLE_INDEX_MAX_V])||
		   (EepromPara.EepromParaS.SystemInformationP.PSN[TempI] < Char_Para_Table[19][PARATABLE_INDEX_MIN_V])||(EepromPara.EepromParaS.SystemInformationP.PSN[TempI] == 0xFF))
		{
			EepromPara.EepromParaS.SystemInformationP.PSN[TempI] = Char_Para_Table[19][PARATABLE_INDEX_DEFAULT_V];
		}
	}
	
	for(TempI = 0; TempI < DESCRIPTOR_LENGTH; TempI++)
	{
		TempPara_Address = Char_Para_Table[20][PARATABLE_INDEX_ADDRESS];
		if((EepromPara.EepromParaS.SystemInformationP.PDescriptor[TempI] > Char_Para_Table[20][PARATABLE_INDEX_MAX_V])||
		   (EepromPara.EepromParaS.SystemInformationP.PDescriptor[TempI] < Char_Para_Table[20][PARATABLE_INDEX_MIN_V])||(EepromPara.EepromParaS.SystemInformationP.PDescriptor[TempI] == 0xFF))
		{
			EepromPara.EepromParaS.SystemInformationP.PDescriptor[TempI] = Char_Para_Table[20][PARATABLE_INDEX_DEFAULT_V];
		}
	}
	
	for(TempI = 0; TempI < TAG_LENGTH; TempI++)
	{
		TempPara_Address = Char_Para_Table[21][PARATABLE_INDEX_ADDRESS];
		if((EepromPara.EepromParaS.SystemInformationP.PTag[TempI] > Char_Para_Table[21][PARATABLE_INDEX_MAX_V])||
		   (EepromPara.EepromParaS.SystemInformationP.PTag[TempI] < Char_Para_Table[21][PARATABLE_INDEX_MIN_V])||(EepromPara.EepromParaS.SystemInformationP.PTag[TempI] == 0xFF))
		{
			EepromPara.EepromParaS.SystemInformationP.PTag[TempI] = Char_Para_Table[21][PARATABLE_INDEX_DEFAULT_V];
		}
	}
	RecordPWMCurrentPara();
}

void	SaveModifyParaInfo(void)
{
	if(SaveModifyParaInfoFlag == ENABLE)
	{
		SaveModifyParaInfoFlag = DISABLE;
		WriteEeprom(PARAMODIFYINFOINDEX+ModifyParaInfo_Count*sizeof(RecordModifyPara),(uint8_t*)RecordModifyPara.RecordModifyParaC,sizeof(RecordModifyPara));
		ModifyParaInfo_Count++;
		if(ModifyParaInfo_Count >= MODIFYPARAINFO_MAXCOUNT)
		{
			ModifyParaInfo_Count = 0;
		}
		RecordRealPerfToFRAM(ModifyParaInfo_Count,MODIFYPARINFO_P);		
	}
	
	if((FakeInfo_FlagOfChange == ENABLE_WRITEEEPROM)&&(FakeInfo_Index < FAKEINFO_MAX_INDEX))
	{
		WriteEeprom(FAKEINFO_EEPROM_ADDRESS+EEPROM_PAGESIZE*FakeInfo_Index,((uint8_t*)(FakeWave)+EEPROM_PAGESIZE*FakeInfo_Index),EEPROM_PAGESIZE);
		FakeInfo_Index++;
		if(FakeInfo_Index == FAKEINFO_MAX_INDEX)
		{
			FakeInfo_FlagOfChange = DISABLE_WRITEEEPROM;
			FakeInfo_Index = 0;
		}
	}
}

void	ClearModifyParaInfo(void)
{
	uint8_t TempC;
	ModifyParaInfo_Count = 0;
	RecordModifyPara.RecordModifyParaS.ModfiyTime[0] = 0;
	RecordModifyPara.RecordModifyParaS.ModfiyTime[1] = 0;
	RecordModifyPara.RecordModifyParaS.BeforeModifyParaV.FloatUnion = 0;
	RecordModifyPara.RecordModifyParaS.UpdateModifyParaV.FloatUnion = 0;
	RecordModifyPara.RecordModifyParaS.ModfiyComMode = 0;
	for(TempC = 0; TempC < MODIFYPARAINFO_MAXCOUNT; TempC++)
	{
		SaveModifyParaInfoFlag = ENABLE;
		SaveModifyParaInfo();
	}
	RecordRealPerfToFRAM(ModifyParaInfo_Count,MODIFYPARINFO_P);		
}

void	InitPara(void)
{
	uint16_t TempI;
	SaveByteNumber = 0;
	StartAddressRom = 0;
	AllUserPara_HandleMethod = USERPARA_NO_OPS;
	ReadFake_Status = 1;
	FakeInfo_Index = 0;
	CRCParaAddress_FlagOfChange = DISABLE_WRITEEEPROM;
	
	for(TempI = 0; TempI < CHARPARANUMBERS; TempI++)
	{
		CharPara_FlagOfChange[TempI] = DISABLE_WRITEEEPROM;
	}
	for(TempI = 0; TempI < FLOATPARANUMBERS; TempI++)
	{
		FloatPara_FlagOfChange[TempI] = DISABLE_WRITEEEPROM;
	}
	
	ReadMeterPara();
	PWM4mABias = 4;
	PWM18mABias = 18;
	if(EepromPara.EepromParaS.Eeprom_InitFlagI != NOT_FIRST_TIME)
	{
		EepromPara.EepromParaS.Eeprom_InitFlagI = NOT_FIRST_TIME;
		ClearModifyParaInfo();
		ClearAllPerf();
		InitSystemInformation();
		InitParaDefault();
		HAL_Delay(WRITEEEPROM_DELAY);
		EepromPara.EepromParaS.ExpandPara.CharPara[0] = 0;
		for(TempI = 0; TempI < sizeof(EepromPara.EepromParaS); TempI= TempI+EEPROM_PAGESIZE)
		{
			WriteEeprom(TempI,(uint8_t*)(EepromPara.EepromParaC+TempI),EEPROM_PAGESIZE);
			HAL_Delay(WRITEEEPROM_DELAY);
		}
	}
	CheckPara();
	StringToInt_ProductType();
	CalculateParaCRC();
	StringToInt_ProductType();
	StringToInt_ProductSL();
	
	do
	{//读虚假回波信息
		ReadEeprom(FAKEINFO_EEPROM_ADDRESS+128*FakeInfo_Index,((uint8_t*)(FakeWave)+128*FakeInfo_Index),128);		
		FakeInfo_Index++;
	}
	while(FakeInfo_Index < 16);
	for(TempI = 0; TempI < SAMPLE_DATA_LENGTH; TempI++)
	{
		if(FakeWave[TempI] > FAKEECHO_MAXVALURE)
		{
			FakeWave[TempI] = 0;
		}
	}
	Update_LPUART1_UART_Para();
	UnitConv_Ratio = UnitPara_Ratio[EepromPara.EepromParaS.BasicPara.PVUnit];
	FakeInfo_Index = 0;
	WriteEEpromInterval = 0;
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

void	DeliverModifyParaInfo(uint8_t CurrentModifyComMode,uint16_t ParaAddress)
{
	GetMeterCurrentTime(RecordModifyPara.RecordModifyParaS.ModfiyTime);
	RecordModifyPara.RecordModifyParaS.ModfiyComMode = CurrentModifyComMode;
	RecordModifyPara.RecordModifyParaS.ModfiyParaAddress = ParaAddress;
	SaveModifyParaInfoFlag = ENABLE;
//	SaveModifyParaInfo();
}

void	SavePara_FloatType(uint8_t TempModifyComMode,uint16_t ParaAddress, float *ParaValueP)
{
//	uint16_t TempI;
	uint16_t TempPara_Address;
	uint8_t TempC;
	union
	{
		float TempF;
		unsigned char TempChar[4];
	}TempFloatToChar;
	
	if((EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)&&(ParaAddress < FLOATPARANUMBERS))
	{
		RecordPerfToFRAM(MODFYPARATIMES_TOTAL);
		TempPara_Address = (uint16_t)(Float_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]);
		TempFloatToChar.TempF = *ParaValueP;
		
		switch(ParaAddress)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 6:
			case 7:
			case 8:
			case 12:
			case 13:
			case 14:
			case 15:
			case 17:
			case 21:
			case 22:
			case 24:
			case 25:
			case 31:
				TempFloatToChar.TempF = TempFloatToChar.TempF/UnitConv_Ratio;
			break;
			default:break;
		}
		
		if((TempFloatToChar.TempF <= Float_Para_Table[ParaAddress][PARATABLE_INDEX_MAX_V])&&
		   (TempFloatToChar.TempF >= Float_Para_Table[ParaAddress][PARATABLE_INDEX_MIN_V]))
		{
			FloatPara_FlagOfChange[ParaAddress] = ENABLE_WRITEEEPROM;
			for(TempC = 0; TempC < 4; TempC++)
			{
				RecordModifyPara.RecordModifyParaS.BeforeModifyParaV.CharUnion[TempC] = EepromPara.EepromParaC[TempPara_Address+TempC];
				EepromPara.EepromParaC[TempPara_Address+TempC] = TempFloatToChar.TempChar[TempC];
				RecordModifyPara.RecordModifyParaS.UpdateModifyParaV.CharUnion[TempC] = EepromPara.EepromParaC[TempPara_Address+TempC];
			}
			if(ParaAddress == 0)
			{
				if(EepromPara.EepromParaS.RangeValue.LowerRange > UserMaxLowerRange)
				{
					EepromPara.EepromParaS.RangeValue.LowerRange = UserMaxLowerRange;
					RecordModifyPara.RecordModifyParaS.UpdateModifyParaV.FloatUnion = UserMaxLowerRange;
				}
				FloatPara_FlagOfChange[7] = ENABLE_WRITEEEPROM;
				EepromPara.EepromParaS.RangeLimit.LowerRange = EepromPara.EepromParaS.RangeValue.LowerRange+0.5;
				if(EepromPara.EepromParaS.RangeLimit.LowerRange > Float_Para_Table[7][PARATABLE_INDEX_MAX_V])
				{
					EepromPara.EepromParaS.RangeLimit.LowerRange = Float_Para_Table[7][PARATABLE_INDEX_MAX_V];
				}
				SetLcdLCDCurveEnd(EepromPara.EepromParaS.RangeLimit.LowerRange);
			}
			if(RecordModifyPara.RecordModifyParaS.BeforeModifyParaV.FloatUnion != RecordModifyPara.RecordModifyParaS.UpdateModifyParaV.FloatUnion)
			{
				DeliverModifyParaInfo(TempModifyComMode,ParaAddress+1000);
			}
		}
		/*------*/
//		UpdateConfigIWR1443Para();
		ParaConvert();
		CalculateParaCRC();
	}
}

void	SavePara_CharType(uint8_t TempModifyComMode,uint16_t ParaAddress, uint8_t ParaValue)
{
	if((EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)&&(ParaAddress < CHARPARANUMBERS))
	{
		RecordPerfToFRAM(MODFYPARATIMES_TOTAL);
		if((ParaValue <= Char_Para_Table[ParaAddress][PARATABLE_INDEX_MAX_V])&&
		  (ParaValue >= Char_Para_Table[ParaAddress][PARATABLE_INDEX_MIN_V]))
		{
			RecordModifyPara.RecordModifyParaS.BeforeModifyParaV.FloatUnion = (float)(EepromPara.EepromParaC[Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]]);
			EepromPara.EepromParaC[Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]] = ParaValue;
			RecordModifyPara.RecordModifyParaS.UpdateModifyParaV.FloatUnion = (float)(EepromPara.EepromParaC[Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]]);
			CharPara_FlagOfChange[ParaAddress] = ENABLE_WRITEEEPROM;
//			WriteEeprom(Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS],
//			    (uint8_t*)(EepromPara.EepromParaC+Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]),1);
			if(RecordModifyPara.RecordModifyParaS.BeforeModifyParaV.FloatUnion != RecordModifyPara.RecordModifyParaS.UpdateModifyParaV.FloatUnion)
			{
				DeliverModifyParaInfo(TempModifyComMode,ParaAddress);
			}
		}
		UpdateConfigIWR1443Para();
		CalculateParaCRC();
		if(ParaAddress == 8)
		{//改变LCD通信波特率
			Update_LPUART1_UART_Para();
		}
		UnitConv_Ratio = UnitPara_Ratio[EepromPara.EepromParaS.BasicPara.PVUnit];
	}
}

void	SavePara_StringType(uint16_t ParaAddress, uint8_t *ParaValueP)
{
	uint16_t TempC;
	if((EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)&&(ParaAddress < CHARPARANUMBERS))
	{
		RecordPerfToFRAM(MODFYPARATIMES_TOTAL);
		for(TempC = 0; TempC < Char_Para_Table[ParaAddress][PARATABLE_INDEX_BYTES]; TempC++)
		{
		//避免最大值最小值屏蔽掉这几个选项
                  if((ParaAddress == PTagIndex)
                     ||(ParaAddress == PDescriptorIndex)
                       ||(ParaAddress == PDateIndex)
                         ||(ParaAddress == FinalAsmNoIndex)
                          ||(ParaAddress == ConfigChangeCounter))
                  {
                    EepromPara.EepromParaC[Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS]+TempC] = *ParaValueP;
                    ParaValueP++;
                  }
                  else
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
		}

		Write_Para_Multibyte(Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS],Char_Para_Table[ParaAddress][PARATABLE_INDEX_BYTES]);
		CalculateParaCRC();
		if(ParaAddress == 21)
		{
			Set_BLTooth_Adv_Data(1);
		}
		StringToInt_ProductType();
		StringToInt_ProductSL();
	}
}

void	SavePara_LineType(uint16_t ParaAddress, float *ParaValueP)
{
	uint16_t TempPara_Address;
	if(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)
	{
		RecordPerfToFRAM(MODFYPARATIMES_TOTAL);
		EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][0] = *ParaValueP;
		ParaValueP++;
		EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][1] = *ParaValueP;

		TempPara_Address = ((uint8_t volatile*)&EepromPara.EepromParaS.LinearPara.LinearParaFB-EepromPara.EepromParaC)+(ParaAddress-55)*8;
		Write_Para_Multibyte(TempPara_Address,8);
		CalculateParaCRC();
	}
}

void	SavePara_PointsNum(uint8_t ParaValue)
{
	uint16_t TempPara_Address;
	if(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)
	{
		RecordPerfToFRAM(MODFYPARATIMES_TOTAL);
		TempPara_Address = ((uint8_t volatile*)&EepromPara.EepromParaS.LinearPara.LinePointsNum-EepromPara.EepromParaC);
		EepromPara.EepromParaS.LinearPara.LinePointsNum = ParaValue;
		WriteEeprom(TempPara_Address,&ParaValue,BYTE_LENGTH);		
		CalculateParaCRC();
	}	
}

uint8_t	GetPara_PointsNum(void)
{
	return(EepromPara.EepromParaS.LinearPara.LinePointsNum);
}

uint8_t *GetPara_Char(uint16_t ParaAddress)
{
	uint16_t TempPara_Address;
	TempPara_Address = (uint16_t)Char_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS];
	return(&EepromPara.EepromParaC[TempPara_Address]);
}
//Mr.刘的代码，封装最大值最小值获取函数
/* 获取字符型变量最大值 */
uint8_t GetPara_CharMax(uint16_t ParaAddress)
{
	return Char_Para_Table[ParaAddress][PARATABLE_INDEX_MAX_V];
}
/* 获取字符型变量最小值 */
uint8_t GetPara_CharMin(uint16_t ParaAddress)
{
	return Char_Para_Table[ParaAddress][PARATABLE_INDEX_MIN_V];
}
/* 获取字符型变量最大值 */
float GetPara_FloatMax(uint16_t ParaAddress)
{
	return Float_Para_Table[ParaAddress][PARATABLE_INDEX_MAX_V];
}
/* 获取字符型变量最小值 */
float GetPara_FloatMin(uint16_t ParaAddress)
{
	return Char_Para_Table[ParaAddress][PARATABLE_INDEX_MIN_V];
}

/* 单位转换系数获取 */
float UnitParaRatioGet(uint8_t PVUnit)
{
  return UnitPara_Ratio[PVUnit];
}
void	GetPara_Float(uint8_t *TempDataBuffer,uint16_t ParaAddress)
{
	uint8_t TempC;
	uint16_t TempPara_Address;
	TempPara_Address = (uint16_t)Float_Para_Table[ParaAddress][PARATABLE_INDEX_ADDRESS];
	
	for(TempC = 0; TempC < 4; TempC++)
	{
		PartBuffer_Out[TempC] = EepromPara.EepromParaC[TempPara_Address+TempC];
	}
	
		switch(ParaAddress)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 6:
			case 7:
			case 8:
			case 12:
			case 13:
			case 14:
			case 15:
			case 17:
			case 21:
			case 22:
			case 24:
			case 25:
			case 31:
				*(float*)(&PartBuffer_Out[0]) = *(float*)(&PartBuffer_Out[0])*UnitConv_Ratio;
			break;
			default:break;
		}
	for(TempC = 0; TempC < 4; TempC++)
	{
		TempDataBuffer[TempC] = PartBuffer_Out[TempC];
	}
//	return(&PartBuffer_Out[0]);
}

void	ReadParaL_LCD(uint16_t ParaAddress, float *ParaValueP)
{
	*ParaValueP = EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][0];
	ParaValueP++;
	*ParaValueP = EepromPara.EepromParaS.LinearPara.LinearParaFB[ParaAddress-55][1];
}

void	Save_Eeprom_EnablePara_Protect(uint8_t ParaValueP[13])
{
	uint8_t TempC;
	uint8_t TempReceiveData[13];
	
	for(TempC = 0; TempC < 13; TempC++)
	{
		TempReceiveData[TempC] = ParaValueP[TempC];
	}
	for(TempC = 0; TempC < 6; TempC++)
	{
		TempReceiveData[TempC] = ~TempReceiveData[TempC];
		if(TempReceiveData[TempC] != TempReceiveData[TempC+6])
		{
			TempC = 0x5A;
			break;
		}
	}
	if(TempC != 0x5A)
	{
		EepromPara.EepromParaS.Eeprom_EnablePara_Protect = ParaValueP[12]; /* 更新参数写保护功能标志 */
		WriteEeprom(Char_Para_Table[Eeprom_EnablePara_ProtectIndex][PARATABLE_INDEX_ADDRESS],
		    (uint8_t*)(EepromPara.EepromParaC+Char_Para_Table[Eeprom_EnablePara_ProtectIndex][PARATABLE_INDEX_ADDRESS]),BYTE_LENGTH);
	//HAL_I2C_Mem_Read(&hi2c2, (uint16_t)EEPROM_ADDRESS, StartAddressRom, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pData, SaveByteNumber);
	}
}
void	PWM20_PWM18_Current(float ParaValue)
{
	PWM18mABias = PWM4mABias+(ParaValue-PWM4mABias)*0.875;
}

void	Set_PWM18mABias_Value(float ParaValue)
{
	PWM18mABias = ParaValue;
}

void	Set_PWM4mABias_Value(float ParaValue)
{
	PWM4mABias = ParaValue;
}

void	ModifyCurrent(uint8_t TempModifyComMode)
{
	uint16_t TempPara_Address;
	float paraa,parab;
	if((PWM18mABias > PWM4mABias)&&(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT))
	{
		DeliverModifyParaInfo(TempModifyComMode,9003);
		RecordPerfToFRAM(MODFYPARATIMES_TOTAL);
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
		
		RecordPWMCurrentPara();
		CalculateParaCRC();
	}
}

void	SetAllUserPara_HandleMethod(uint8_t TempModifyComMode,uint8_t ParaValue)
{//用户恢复出厂参数或者默认参数、或者保存为出厂参数操作，把变量设置成相应的操作模式
	if(AllUserPara_HandleMethod == 0)
	{
		AllUserPara_HandleMethod = ParaValue;
		if(AllUserPara_HandleMethod == DEFAULTPARATOUSERPARA)
		{
			DeliverModifyParaInfo(TempModifyComMode,9000);
		}
		else if(AllUserPara_HandleMethod == FACTORYPARATOUSERPARA)
		{
			DeliverModifyParaInfo(TempModifyComMode,9001);
		}
		UserParaAddressIndex = 0;
	}
}

uint8_t	GetAllUserPara_HandleMethod(void)
{
	return(AllUserPara_HandleMethod);
}

void	SetSaveFakeInfo(uint8_t TempFakeInfo_Status)
{
	FakeInfo_FlagOfChange = TempFakeInfo_Status;
	FakeInfo_Index = 0;
}

void	SavePara_WriteEeprom()
{//把用户修改的参数写入到EEPROM芯片里
	for(;CharParaAddress_FlagOfChange_Index < CHARPARANUMBERS; CharParaAddress_FlagOfChange_Index++)
	{
		if(CharPara_FlagOfChange[CharParaAddress_FlagOfChange_Index] == ENABLE_WRITEEEPROM)
		{
			CharPara_FlagOfChange[CharParaAddress_FlagOfChange_Index] = DISABLE_WRITEEEPROM;
			if((CharParaAddress_FlagOfChange_Index >= 18)&&(CharParaAddress_FlagOfChange_Index <= 23))
			{
				Write_Para_Multibyte(Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_ADDRESS],Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_BYTES]);
			}
			else
			{
				WriteEeprom(Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_ADDRESS],
				(uint8_t*)(EepromPara.EepromParaC+Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_ADDRESS]),
									Char_Para_Table[CharParaAddress_FlagOfChange_Index][PARATABLE_INDEX_BYTES]);
			}
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
			if(CRCParaAddress_FlagOfChange == ENABLE_WRITEEEPROM)
			{
				CRCParaAddress_FlagOfChange = DISABLE_WRITEEEPROM;
				Write_Para_Multibyte(((uint8_t volatile*)&EepromPara.EepromParaS.EepromPara_CRC-EepromPara.EepromParaC),2);				
			}
		}
	}
	
}

void	SaveUserPara(void)
{
	if(EepromPara.EepromParaS.Eeprom_EnablePara_Protect == DISABLEPARA_PROTECT)
	{//根据用户设定对参数的操作模式，把相应区域的参数拷贝到另外区域
		if((HAL_GetTick()-WriteEEpromInterval) > WRITEEEPROM_DELAY)
		{
			switch(AllUserPara_HandleMethod)
			{
				case	USERPARATOFACTORPARA://保存为出厂参数
					WriteEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),EEPROM_PAGESIZE);
					UserParaAddressIndex = UserParaAddressIndex+EEPROM_PAGESIZE;
					if((UserParaAddressIndex+EEPROM_PAGESIZE) > sizeof(EepromPara))
					{
						WriteEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),(sizeof(EepromPara)-UserParaAddressIndex));
						AllUserPara_HandleMethod = USERPARA_NO_OPS;
					}
				break;
				case	FACTORYPARATOUSERPARA://恢复出厂参数
//					if(Get_IWR1443_Status() == IWR1443_WAIT_STATUS)
//					{
						ReadEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),EEPROM_PAGESIZE);
						WriteEeprom(UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),EEPROM_PAGESIZE);
						UserParaAddressIndex = UserParaAddressIndex+EEPROM_PAGESIZE;
						if((UserParaAddressIndex+EEPROM_PAGESIZE) > sizeof(EepromPara))
						{
							ReadEeprom(FACTORYPARA_EEPROM_ADDRESS+UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),(sizeof(EepromPara)-UserParaAddressIndex));
							WriteEeprom(UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),(sizeof(EepromPara)-UserParaAddressIndex));
							CheckPara();
							CalculateParaCRC();
							AllUserPara_HandleMethod = USERPARA_NO_OPS;
						}
//					}
				break;
				case	DEFAULTPARATOUSERPARA://恢复默认参数
					InitParaDefault();
					WriteEeprom(UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),EEPROM_PAGESIZE);
					UserParaAddressIndex = UserParaAddressIndex+EEPROM_PAGESIZE;
					if((UserParaAddressIndex+EEPROM_PAGESIZE) > sizeof(EepromPara))
					{
						WriteEeprom(UserParaAddressIndex,(uint8_t*)(EepromPara.EepromParaC+UserParaAddressIndex),(sizeof(EepromPara)-UserParaAddressIndex));
						CalculateParaCRC();
						AllUserPara_HandleMethod = USERPARA_NO_OPS;
					}
				break;
				default://把没有写完到EEPROM芯片的参数继续写入到EEPROM芯片里，知道写完为止
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
			WriteEEpromInterval = HAL_GetTick();
			UnitConv_Ratio = UnitPara_Ratio[EepromPara.EepromParaS.BasicPara.PVUnit];
		}
	}
	else
	{
		AllUserPara_HandleMethod = USERPARA_NO_OPS;
	}
}

void	CheckRamPara(void)
{
	uint16_t TempParaCRC;
	CheckRamPara_Time++;
	if(CheckRamPara_Time >= CHECKRAMPARA_PERIOD)
	{
		CheckRamPara_Time = 0;
		TempParaCRC = HAL_CRC_Calculate(&hcrc,(uint32_t *)(EepromPara.EepromParaC),sizeof(EepromPara)-4);
		if(TempParaCRC != EepromPara.EepromParaS.EepromPara_CRC)
		{//RAM 报错
			RecordPerfToFRAM(RAMERRORTIMES);
			ReadMeterPara();
		}
	}
}

uint8_t	GetComMethod(void)
{
	return(EepromPara.EepromParaS.ComPara.BaudRate);
}

uint8_t	GetComAddress(void)
{
	return(EepromPara.EepromParaS.ComPara.ComAddress);
}

uint8_t	GetComAddress_485(void)
{
	return(EepromPara.EepromParaS.ComPara.ComAddress_485);
}

void	GetModifyParaInfoP(float Fram_ModifyParaInfo)
{
	ModifyParaInfo_Count = (uint8_t)(Fram_ModifyParaInfo);
	if(ModifyParaInfo_Count >= MODIFYPARAINFO_MAXCOUNT)
	{
		ModifyParaInfo_Count = 0;
	}
	RecordRealPerfToFRAM(ModifyParaInfo_Count,MODIFYPARINFO_P);		
}

void	ReadModifyParaInfo(uint8_t *ModifyParaInfoEEprom,uint8_t ReadModifyParaInfo_Index)
{
	ReadEeprom(PARAMODIFYINFOINDEX+ReadModifyParaInfo_Index*sizeof(RecordModifyPara), (uint8_t *)ModifyParaInfoEEprom,sizeof(RecordModifyPara));
//	ReadModifyParaInfo_Count++;
//	if(ReadModifyParaInfo_Count >= MODIFYPARAINFO_MAXCOUNT)
//	{
//		ReadModifyParaInfo_Count = 0;
//	}
}

uint16_t GetRFWorkTimeMax(void)
{
	uint16_t TempI;
	if((EepromPara.EepromParaS.ExpandPara.FloatPara[15] < 160)||(EepromPara.EepromParaS.ExpandPara.FloatPara[15] > 8600))
	{
		TempI = 200;
	}
	else
	{
		TempI = (uint16_t)(EepromPara.EepromParaS.ExpandPara.FloatPara[15]);
	}
	return(TempI);
}

float	GetVconMinVolage(void)
{
	return(EepromPara.EepromParaS.ExpandPara.FloatPara[5]);
}

float	GetUnitConv_Ratio(void)
{
	return(UnitConv_Ratio);
}

float	GetRFGainPara(void)
{
	return(EepromPara.EepromParaS.ExpandPara.FloatPara[13]);
}
