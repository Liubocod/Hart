#include	"main.h"
#include	"usart.h"
#include	"ComLCDHandle.h"
#include 	"DistanceDataHandle.h"
#include	"EchoHandle.h"
#include	"Eeprom.h"
#include	"IWR1443.h"
#include	"LCD485.h"
#include	"perf_management.h"

uint8_t RTBuffer_LCD485[MAX_LCD485_BUFFER];
uint8_t TxRxCount_LCD485;
uint8_t TransitDataNumber_ThisTime,ReceiveDataNumber_ThisTime;
uint8_t LCD485ComStatus;
uint8_t AutoTransmit_Flag;
uint16_t StartAddress;
uint16_t LCD485WorkCount;
uint32_t LCD485ReceiveCompletTime;

/*--------------------------MODBUS CRC计算----------------------------------------------*/
uint16_t	crc_check(uint8_t pack_byte)
{
	uint16_t polyn_m = 0xA001;
	uint8_t crc_i,crc_j;
	uint16_t crc = 0xFFFF;
	for(crc_i=0;crc_i<pack_byte;crc_i++)
	{
		crc ^= RTBuffer_LCD485[crc_i];
		for(crc_j=0;crc_j<8;crc_j++)
		{
			if((crc&0x01)==0x01)
			{
				crc>>=1;
				crc^=polyn_m;
			}
			else
				crc>>=1;
		}
	}
	return( crc );
}

void	FloatToChar_RT485(float FloatVar,uint8_t DataIndex)
{
	uint8_t *point_Float;
	point_Float = (uint8_t *)&FloatVar;
	
	RTBuffer_LCD485[DataIndex] = point_Float[3];
	RTBuffer_LCD485[DataIndex+1] = point_Float[2];
	RTBuffer_LCD485[DataIndex+2] = point_Float[1];
	RTBuffer_LCD485[DataIndex+3] = point_Float[0];	
}

void	EnableLCD485Receive(void)
{
	HAL_UART_Receive_IT(&hlpuart1, (uint8_t *)RTBuffer_LCD485, 20);
}

void	SendMasterVariable(void)
{
	uint8_t TempC,TempD;
	uint16_t TempI;
	uint16_t TempRegisterAddress;
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;

	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	TempI = RTBuffer_LCD485[5]+StartAddress;
	TempD = 0;
	for(TempRegisterAddress = StartAddress; TempRegisterAddress < TempI; TempRegisterAddress = TempRegisterAddress+2)
	{//上位机读数据
		TempFloatToChar.TempF = HartReadMasterVariable((TempRegisterAddress-500)/2);
		for(TempC = 0; TempC < 4; TempC++)
		{
			RTBuffer_LCD485[4*TempD+TempC+3] = TempFloatToChar.TempChar[TempC];
		}
		TempD++;
	}
}

/*485通信，发送回波曲线数据、距离测量值（浮点数）、频率、物料回波位置、物料回波信噪比、物料回波幅度、物料回波分辨率*/
void	SendCurveInformation(void)
{
	uint8_t	TempRegisterAddress;
	uint8_t TempC;
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;
//	for(TempC = 3; TempC < 103; TempC++)
//		RTBuffer_LCD485[TempC] = LcdEcho[TempC-3];
//	ReadCurveData(&RTBuffer_LCD485[3]);
	for(TempC = 3; TempC < 103; TempC++)
	{
		RTBuffer_LCD485[TempC] = LcdEcho[TempC-3];
	}
	TempFloatToChar.TempF = HartReadMasterVariable(1);
	for(TempC = 103; TempC < 107; TempC++)
	{
		RTBuffer_LCD485[TempC] = TempFloatToChar.TempChar[TempC-103];
	}
	TempFloatToChar.TempF = GetBestEchoAmplitude();

	for(TempC = 107; TempC < 111; TempC++)
	{
		RTBuffer_LCD485[TempC] = TempFloatToChar.TempChar[TempC-107];
	}
	RTBuffer_LCD485[111] = GetMarkBestWavePostion();
	RTBuffer_LCD485[112] = GetShockWaveInf();
	RTBuffer_LCD485[113] = (uint8_t)(GetBestEchoAmplitude()/AMPLITUDE_COV_SAMPLE_255);
	RTBuffer_LCD485[114] = (uint8_t)(GetBestEchoDistinctiveness());
	
	TempRegisterAddress = (uint8_t)((StartAddress-800)*2);
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	for(TempC = 3; TempC < TransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_LCD485[TempC] = RTBuffer_LCD485[TempC+TempRegisterAddress];
	}
	SetCurveStart_EndPoint(0,0);
}

void	SendPara_Char(void)
{
	uint8_t TempC;
	uint16_t TempRegisterAddress;
	
	RTBuffer_LCD485[3] = 0;
	RTBuffer_LCD485[4] = *GetPara_Char(5);
	for(TempC = 1; TempC < 5; TempC++)
	{
		RTBuffer_LCD485[4+TempC*2] = *GetPara_Char(TempC-1);
		RTBuffer_LCD485[3+TempC*2] = 0;
	}
	for(TempC = 7; TempC < 17; TempC++)
	{
		RTBuffer_LCD485[4+(TempC-2)*2] = *GetPara_Char(TempC);
		RTBuffer_LCD485[3+(TempC-2)*2] = 0;
	}
	RTBuffer_LCD485[34] = GetAllUserPara_HandleMethod();	
	RTBuffer_LCD485[33] = 0;	
	for(TempC = 0; TempC < DATE_LENGTH; TempC++)
	{/*时间*/
		RTBuffer_LCD485[35+TempC] = *(GetPara_Char(18)+TempC);
	}
	for(TempC = 0; TempC < SENSOR_SN_LENGTH; TempC++)
	{
		RTBuffer_LCD485[43+TempC] = *(GetPara_Char(19)+TempC);
	}
	for(TempC = 0; TempC < DESCRIPTOR_LENGTH; TempC++)
	{
		RTBuffer_LCD485[53+TempC] = *(GetPara_Char(20)+TempC);
	}
	RTBuffer_LCD485[69] = VERSION0;
	RTBuffer_LCD485[70] = VERSION1;
	RTBuffer_LCD485[71] = VERSION2;
	RTBuffer_LCD485[72] = 0;
	RTBuffer_LCD485[73] = 0;
	RTBuffer_LCD485[74] = *GetPara_Char(25);
	RTBuffer_LCD485[75] = 0;
	RTBuffer_LCD485[76] = *GetPara_Char(24);
	RTBuffer_LCD485[77] = 0;
	RTBuffer_LCD485[78] = *GetPara_Char(23);
	
	RTBuffer_LCD485[79] = (uint8_t)(Get_IWR1443_Version());
	RTBuffer_LCD485[80] = (uint8_t)(Get_IWR1443_Version()>>8);
	RTBuffer_LCD485[81] = *GetPara_Char(28);
	RTBuffer_LCD485[82] = *GetPara_Char(26);
	RTBuffer_LCD485[83] = *GetPara_Char(29);
	RTBuffer_LCD485[84] = *GetPara_Char(27);
	
	TempRegisterAddress = (uint8_t)((StartAddress-9)*2);
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	for(TempC = 3; TempC < TransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_LCD485[TempC] = RTBuffer_LCD485[TempC+TempRegisterAddress];
	}
}

void	SendMeter_TAG(void)
{
	uint8_t TempC;
//	uint16_t TempRegisterAddress;
	for(TempC = 0; TempC < TAG_LENGTH; TempC++)
	{
		RTBuffer_LCD485[3+TempC] = *(GetPara_Char(21)+TempC);
	}
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
}

void	SendExpandPara_Char(void)
{
	uint8_t TempC;
	uint16_t TempRegisterAddress;
	for(TempC = 0; TempC < EXPANDCHARPARANUMBERS; TempC++)
	{
		RTBuffer_LCD485[4+TempC*2] = *GetPara_Char(EXPANDCHARPARA_STARTADDRESS_485+TempC);
		RTBuffer_LCD485[3+TempC*2] = 0;
	}
	
	TempRegisterAddress = (uint8_t)((StartAddress-0x1000)*2);
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	for(TempC = 3; TempC < TransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_LCD485[TempC] = RTBuffer_LCD485[TempC+TempRegisterAddress];
	}
}

void	SendPerfData(void)
{
	uint8_t TempParaNumber,TempC;
	uint16_t TempRegisterAddress;
	TempParaNumber = (uint8_t)((StartAddress-1000)/2);
	TempRegisterAddress = RTBuffer_LCD485[5]/2;
	
	for(TempC = 0; TempC < TempRegisterAddress; TempC++)
	{
		PerfDataRead(TempParaNumber,(float *)(&RTBuffer_LCD485[3+TempC*4]));
		TempParaNumber++;
	}
	
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
}

void	SendPara_Float(void)
{
	uint8_t TempParaNumber,TempC;
	uint16_t TempRegisterAddress;
	for(TempParaNumber = 0; TempParaNumber < 9; TempParaNumber++)
	{
		GetPara_Float(&RTBuffer_LCD485[3+TempParaNumber*4],TempParaNumber);
	}
	
	for(TempParaNumber = 10; TempParaNumber < 21; TempParaNumber++)
	{
		GetPara_Float(&RTBuffer_LCD485[3+TempParaNumber*4],TempParaNumber);
	}

	GetPara_Float(&RTBuffer_LCD485[87],9);
	TempRegisterAddress = (StartAddress-50)*2;
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	for(TempC = 3; TempC < TransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_LCD485[TempC] = RTBuffer_LCD485[TempC+TempRegisterAddress];
	}
}

void	SendExpandPara_Float(void)
{
	uint8_t TempC;
	uint16_t TempI;
	uint16_t TempRegisterAddress;
	
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	TempRegisterAddress = (StartAddress-0x2000)/2;
	TempI = RTBuffer_LCD485[5]/2+TempRegisterAddress;
	TempC = 0;
	for(; TempRegisterAddress < TempI; TempRegisterAddress++)
	{//上位机读数据
		GetPara_Float(&RTBuffer_LCD485[3+TempC*4],EXPANDFLOATPARA_STARTADDRESS_485+TempRegisterAddress);
		TempC++;
	}
}

/*485通信，根据接收到上位机的命令帧，保存单字节参数*/
void	SavePara485_Char(void)
{
	if(StartAddress < 14)
	{
		if(StartAddress == 9)
		{
			SavePara_CharType(MODIFYPARAMODE_LCD485,5,RTBuffer_LCD485[8]);
		}
		else if(StartAddress >= 10)
		{
			SavePara_CharType(MODIFYPARAMODE_LCD485,StartAddress-10,RTBuffer_LCD485[8]);
		}
	}
	else if((StartAddress >= 14)&&(StartAddress <= 46))
	{
//		if(StartAddress != 24)
			StartAddress = StartAddress+3;
		switch(StartAddress)
		{
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
			case 25:
			case 26:
				SavePara_CharType(MODIFYPARAMODE_LCD485,StartAddress-10,RTBuffer_LCD485[8]);
			break;
			case 27:
				SetAllUserPara_HandleMethod(MODIFYPARAMODE_LCD485,RTBuffer_LCD485[8]);
			break;
			case 28:
				SavePara_StringType(StartAddress-10,&RTBuffer_LCD485[7]);
			break;
			case 32:
				SavePara_StringType(StartAddress-13,&RTBuffer_LCD485[7]);
			break;
			case 37:
				SavePara_StringType(StartAddress-17,&RTBuffer_LCD485[7]);
			break;
			case 47:
				Save_Eeprom_EnablePara_Protect(&RTBuffer_LCD485[7]);
			break;
			case 48:
				SavePara_CharType(MODIFYPARAMODE_LCD485,StartAddress-24,RTBuffer_LCD485[8]);
			break;
			case 49:
				SavePara_CharType(MODIFYPARAMODE_LCD485,StartAddress-26,RTBuffer_LCD485[8]);
			break;
			default:break;
		}
	}
	else if(StartAddress > 46)
	{
		switch(StartAddress)
		{
			case 48:
				SavePara_CharType(MODIFYPARAMODE_LCD485,StartAddress-22,RTBuffer_LCD485[8]);
			break;
			case 49:
//				SavePara_StringType(StartAddress-22,&RTBuffer_LCD485[7]);
				SavePara_CharType(MODIFYPARAMODE_LCD485,StartAddress-22,RTBuffer_LCD485[8]);
			break;
			default:break;
		}
	}
}

/*485通信，根据接收到上位机的命令帧，保存4字节浮点数参数*/
void	SavePara485_Float(void)
{
	switch(StartAddress)
	{
		case 50:
		case 52:
		case 60:
		case 62:
		case 64:
		case 66:
		case 74:
		case 76:
		case 78:
		case 80:
		case 82:
		case 84:
		case 86:
		case 88:
		case 90:
			SavePara_FloatType(MODIFYPARAMODE_LCD485,(StartAddress-50)/2, (float*)(&RTBuffer_LCD485[7]));
		break;
		case 58:
			SavePara_FloatType(MODIFYPARAMODE_LCD485,4,(float*)(&RTBuffer_LCD485[7]));
			
			if(RTBuffer_LCD485[2] >= 8)
			{
				SavePara_FloatType(MODIFYPARAMODE_LCD485,5, (float*)(&RTBuffer_LCD485[11]));
			}
		break;
		case 68:
			if(*(float*)(&RTBuffer_LCD485[7]) == 1)
			{
				ModifyCurrent(MODIFYPARAMODE_LCD485);
				*(float*)(&RTBuffer_LCD485[7]) = 18;
			}
			if((*(float*)(&RTBuffer_LCD485[7]) < MINCURRENT_SET)||(*(float*)(&RTBuffer_LCD485[7]) > MAXCURRENT_SET))
				*(float*)(&RTBuffer_LCD485[7]) = 0;
			SetOutCurrentVal(*(float*)(&RTBuffer_LCD485[7]));
		break;
		case 70:
			Set_PWM4mABias_Value(*(float*)(&RTBuffer_LCD485[7]));
		break;
		case 72:
			Set_PWM18mABias_Value(*(float*)(&RTBuffer_LCD485[7]));
		break;
		case 92:
			SavePara_FloatType(MODIFYPARAMODE_LCD485,9, (float*)(&RTBuffer_LCD485[7]));
		break;
		default:break;
	}
}

void	HandleLowAdressRegister_10(void)
{
	uint8_t TempC;
	uint16_t TempInt[8];
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;
	
	TempInt[0] = (uint16_t)(HartReadMasterVariable(1)*100);
	TempInt[1] = (uint16_t)(HartReadMasterVariable(1)*1000);
	TempInt[2] = (uint16_t)(HartReadMasterVariable(0)*100);
	TempInt[3] = (uint16_t)(HartReadMasterVariable(0)*1000);
	GetPara_Float(&(TempFloatToChar.TempChar[0]),0);
	TempInt[4] = (uint16_t)(TempFloatToChar.TempF*100);
	GetPara_Float(&(TempFloatToChar.TempChar[0]),1);
	TempInt[5] = (uint16_t)(TempFloatToChar.TempF*100);
	TempInt[6] = (uint16_t)(HartReadMasterVariable(2));
	TempInt[7] = (uint16_t)(HartReadMasterVariable(3)*100);
	for(TempC = 0; TempC < 8; TempC++)
	{
		RTBuffer_LCD485[3+2*TempC] = TempInt[TempC]>>8;
		RTBuffer_LCD485[4+2*TempC] = TempInt[TempC];
	}
	RTBuffer_LCD485[19] = ReadMeterWorkStatus();
	
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	for(TempC = 3; TempC < TransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_LCD485[TempC] = RTBuffer_LCD485[TempC+StartAddress*2];
	}
}

void	ReadRegister_Vega(void)
{
	uint8_t TempParaNumber,TempC;
	uint16_t TempI;
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;
	
	if((StartAddress >= 100)&&(StartAddress < 118))
	{
		switch(StartAddress)
		{
			case 106:
				TempFloatToChar.TempF = HartReadMasterVariable(1);
				for(TempC = 3; TempC < 7; TempC++)
				{
					RTBuffer_LCD485[TempC] = TempFloatToChar.TempChar[TempC-3];
				}
				TempI = (StartAddress-106)*2;
			break;
			default:
			break;
		}
	}
	else if((StartAddress >= 200)&&(StartAddress < 3000))
	{
		RTBuffer_LCD485[4] = *GetPara_Char(7);
		RTBuffer_LCD485[3] = 0;
		RTBuffer_LCD485[6] = *GetPara_Char(8);
		RTBuffer_LCD485[5] = 0;
		TempI = (StartAddress-200)*2;
	}
	else if((StartAddress >= 3200)&&(StartAddress < 3800))
	{
		switch(StartAddress)
		{
			case 3401:
			case 3403:
				for(TempParaNumber = 0; TempParaNumber < 2; TempParaNumber++)
				{
					GetPara_Float(&(RTBuffer_LCD485[3+TempParaNumber*4]),TempParaNumber);
				}
				TempI = (StartAddress-3401)*2;
			break;
			default:
			break;
		}
	}
	
	TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	for(TempC = 3; TempC < TransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_LCD485[TempC] = RTBuffer_LCD485[TempC+TempI];
	}
}

void	WriteRegister_Vegar(void)
{
	
	switch(StartAddress)
	{
		case 3401:
		case 3403:
			SavePara_FloatType(MODIFYPARAMODE_LCD485,(StartAddress-3401)/2, (float*)(&RTBuffer_LCD485[7]));
		break;
		default:break;
	}
}

void	ModbusProtocol_Vega(void)
{
	uint16_t TempCRCCode;
//	float TempF[3];
	if(*GetPara_Char(23) == 0)
	{
		if((LCD485ComStatus == LCD485READYTRANSMIT)&&((HAL_GetTick()-LCD485ReceiveCompletTime) > INTERVALTIME_MAX)&&(Get_IWR1443_Status() == IWR1443_WAIT_STATUS))
		{
			TempCRCCode = RTBuffer_LCD485[TxRxCount_LCD485-1]+RTBuffer_LCD485[TxRxCount_LCD485]*256;
			if(TempCRCCode == crc_check(TxRxCount_LCD485-1))/*判断CRC校验码对不对*/
			{
				StartAddress = RTBuffer_LCD485[2];
				StartAddress <<= 8;
				StartAddress += RTBuffer_LCD485[3];
				if(RTBuffer_LCD485[MODBUSCOMMAND_INDEX] != 0x10)
				{
					RTBuffer_LCD485[2] = RTBuffer_LCD485[5]*2;
				}
				switch(RTBuffer_LCD485[MODBUSCOMMAND_INDEX])
				{
					case	0x01:
						break;
					case	0x02:
						break;
					case	0x03:/*上位机读仪表参数命令*/
						ReadRegister_Vega();
						break;
					case	0x06:
						WriteRegister_Vegar();
						break;
					case	0x10:
						WriteRegister_Vegar();
						break;
					default:break;
				}
				TempCRCCode = crc_check(TransitDataNumber_ThisTime);
				RTBuffer_LCD485[TransitDataNumber_ThisTime] = TempCRCCode;
				TransitDataNumber_ThisTime++;
				RTBuffer_LCD485[TransitDataNumber_ThisTime] = TempCRCCode>>8;
				TransitDataNumber_ThisTime++;
				LCD485ComStatus = LCD485TRANSMITING;
				TxRxCount_LCD485 = 0;
				HAL_UART_Transmit_IT(&hlpuart1, (uint8_t *)RTBuffer_LCD485, TransitDataNumber_ThisTime);
			}
			else
			{/*CRC出错*/
				LCD485ComStatus = LCD485WAITING;
				RecordPerfToFRAM(COM485ERRORTIMES);
				EnableLCD485Receive();
			}
		}
	}
}

void	ModbusProtocol(void)
{
	uint16_t TempCRCCode;
	float TempF[3];
	if(*GetPara_Char(23) == 0)
	{
	if((LCD485ComStatus == LCD485READYTRANSMIT)&&((HAL_GetTick()-LCD485ReceiveCompletTime) > INTERVALTIME_MAX)&&(Get_IWR1443_Status() == IWR1443_WAIT_STATUS))
	{
		LCD485WorkCount++;
		if(LCD485WorkCount > MAXRESETPERF_COUNT)
		{
			ResetPerfData(COM485ERRORTIMES_TH);
			LCD485WorkCount = 0;
		}
		RecordPerfToFRAM(COM485TIMES_TOTAL);
		TempCRCCode = RTBuffer_LCD485[TxRxCount_LCD485-1]+RTBuffer_LCD485[TxRxCount_LCD485]*256;
		if(TempCRCCode == crc_check(TxRxCount_LCD485-1))/*判断CRC校验码对不对*/
		{
			StartAddress = RTBuffer_LCD485[2];
			StartAddress <<= 8;
			StartAddress += RTBuffer_LCD485[3];
			if(RTBuffer_LCD485[MODBUSCOMMAND_INDEX] != 0x10)
				RTBuffer_LCD485[2] = RTBuffer_LCD485[5]*2;
			
			switch(RTBuffer_LCD485[MODBUSCOMMAND_INDEX])
			{
				case	0x01:
					break;
				case	0x02:
					break;
				case	0x03:/*上位机读仪表参数命令*/
					if(StartAddress < 9)
					{
						HandleLowAdressRegister_10();
					}
					else if(StartAddress < 50)
					{
						SendPara_Char();
					}
					else if(StartAddress < 150)
					{
						SendPara_Float();
					}
					else if((StartAddress >= 500)&&(StartAddress <= 508))
					{
						SendMasterVariable();
					}
					else if(StartAddress == 600)
					{
						SendReadFakeCurveInfo(&RTBuffer_LCD485[3]);
						TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
					}
					else if((StartAddress >= 750)&&(StartAddress < 800))
					{
						SendFakeCurveInfo(&RTBuffer_LCD485[3]);
						TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
					}
					else if((StartAddress >= 800)&&(StartAddress < 1000))
					{
						SendCurveInformation();
					}
					else if((StartAddress >= 1000)&&(StartAddress < 2000))
					{
						SendPerfData();
					}
					else if((StartAddress >= 2000)&&(StartAddress < 2250))
					{
						ReadModifyParaInfo(&RTBuffer_LCD485[3],(StartAddress-2000)/10);
						TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
					}
					else if((StartAddress >= 2300)&&(StartAddress < 3500))
					{
						ReadJumpCurveInfo_485(&RTBuffer_LCD485[3],(StartAddress-2300),RTBuffer_LCD485[2]);
						TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
					}
					else if(StartAddress == 3599)
					{
						RTBuffer_LCD485[3] = 0;
						RTBuffer_LCD485[4] = GetPara_PointsNum();
						TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
					}
					else if((StartAddress >= 3600)&&(StartAddress < 4000))
					{//读线性参数值
						ReadParaL_LCD((StartAddress-3600)/4+55,(float*)(&RTBuffer_LCD485[3]));
						TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
					}
					else if((StartAddress >= 0x1000)&&(StartAddress < 0x2000))
					{
						SendExpandPara_Char();
					}
					else if((StartAddress >= 0x2000)&&(StartAddress < 0x3000))
					{
						SendExpandPara_Float();
					}
					else if(StartAddress == 0x3000)
					{
						SendMeter_TAG();
					}
					else if(StartAddress >= 0x3500)
					{
						Get_MulEcho_Information(RTBuffer_LCD485[2]/8,&RTBuffer_LCD485[3]);
						TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
					}
					break;
				case	0x10:
					RecordPerfToFRAM(MODFYPARATIMES_485);
					if(StartAddress < 9)
					{
					}
					else if(StartAddress <= 49)
					{
						SavePara485_Char();
					}
					else if(StartAddress < 150)
					{
						SavePara485_Float();
					}
					else if(StartAddress == 600)
					{/*曲线操作*/
						TempF[0] = *(float*)(&RTBuffer_LCD485[7]);
						TempF[1] = *(float*)(&RTBuffer_LCD485[11]);
						TempF[2] = *(float*)(&RTBuffer_LCD485[15]);
						CreatFakeCurve(&TempF[0]);						
//						SetCurveStart_EndPoint(*((float*)(&RTBuffer_LCD485[7])),*((float*)(&RTBuffer_LCD485[11])));
					}
					else if(StartAddress == 606)
					{
						SetCurveStart_EndPoint(*((float*)(&RTBuffer_LCD485[7])),*((float*)(&RTBuffer_LCD485[11])));
					}
					else if(StartAddress == 1000)
					{
						ClearAllPerf();
					}
					else if((StartAddress >= 2000)&&(StartAddress < 2250))
					{
						
					}
					else if((StartAddress >= 2300)&&(StartAddress < 3500))
					{
//						SetJumpEchoInfoP(0);
						ClearJumpCurveInfo();
					}
					else if(StartAddress == 3599)
					{
						SavePara_PointsNum(RTBuffer_LCD485[8]);
					}
					else if((StartAddress >= 3600)&&(StartAddress < 4000))
					{//修改线性参数值
						SavePara_LineType((StartAddress-3600)/4+55, (float*)(&RTBuffer_LCD485[7]));
					}
					else if((StartAddress >= 0x1000)&&(StartAddress < 0x2000))
					{
						SavePara_CharType(MODIFYPARAMODE_LCD485,StartAddress-0x1000+EXPANDCHARPARA_STARTADDRESS_485,RTBuffer_LCD485[8]);
					}
					else if((StartAddress >= 0x2000)&&(StartAddress < 0x3000))
					{
						SavePara_FloatType(MODIFYPARAMODE_LCD485,(StartAddress-0x2000)/2+EXPANDFLOATPARA_STARTADDRESS_485, (float*)(&RTBuffer_LCD485[7]));
					}
					else if(StartAddress == 0x3000)
					{
						SavePara_StringType(21,&RTBuffer_LCD485[7]);
					}
					else if(StartAddress == 0x8000)
					{
						
					}
					TransitDataNumber_ThisTime = 6;
					break;
				default:break;
			}
			TempCRCCode = crc_check(TransitDataNumber_ThisTime);
			RTBuffer_LCD485[TransitDataNumber_ThisTime] = TempCRCCode;
			TransitDataNumber_ThisTime++;
			RTBuffer_LCD485[TransitDataNumber_ThisTime] = TempCRCCode>>8;
			TransitDataNumber_ThisTime++;
			LCD485ComStatus = LCD485TRANSMITING;
			TxRxCount_LCD485 = 0;
			HAL_UART_Transmit_IT(&hlpuart1, (uint8_t *)RTBuffer_LCD485, TransitDataNumber_ThisTime);
		}
		else
		{/*CRC出错*/
			LCD485ComStatus = LCD485WAITING;
			RecordPerfToFRAM(COM485ERRORTIMES);
			RecordPerfToFRAM(COM485ERRORTIMES_TH);
			EnableLCD485Receive();
		}
	}
	}
}

static	HAL_StatusTypeDef LCD485_Receive_IT(void)
{
//	uint8_t  TempC;
	uint16_t  uhdata;

	/* Check that a Rx process is ongoing */
	if(hlpuart1.RxState == HAL_UART_STATE_BUSY_RX)
	{
		uhdata = (uint16_t) READ_REG(hlpuart1.Instance->RDR)&0x00FF;
		if(LCD485ComStatus == LCD485WAITING)
		{
			if((uhdata == GetComAddress_485())||(uhdata == 0xAA))
			{
				LCD485ComStatus = LCD485RECEIVING;
				RTBuffer_LCD485[DEVICEADDRESS_INDEX] = uhdata;
				TxRxCount_LCD485 = 0;
			}
//			else
//			{
//			}
		}
		else if(LCD485ComStatus == LCD485RECEIVING)
		{
			TxRxCount_LCD485++;
			RTBuffer_LCD485[TxRxCount_LCD485] = uhdata;
			if(TxRxCount_LCD485 < 7)
			{
				if((RTBuffer_LCD485[MODBUSCOMMAND_INDEX] <= 0x06)&&(RTBuffer_LCD485[MODBUSCOMMAND_INDEX] >= 0x01))
				{
					ReceiveDataNumber_ThisTime = 7;
				}
				else
				{
					if(RTBuffer_LCD485[MODBUSCOMMAND_INDEX] == 0x10)   //person computer write
					{
						if(TxRxCount_LCD485 == 6)
						{
							ReceiveDataNumber_ThisTime = RTBuffer_LCD485[6]+8;
							if(ReceiveDataNumber_ThisTime > MAX_LCD485_BUFFER)
							{
								LCD485ComStatus = LCD485WAITING;
								TxRxCount_LCD485 = 0;
							}
						}
					}
					else
					{
						LCD485ComStatus = LCD485WAITING;
						TxRxCount_LCD485 = 0;
					}
				}
			}
			else
			{
				if(TxRxCount_LCD485 >= MAX_LCD485_BUFFER)
				{
					LCD485ComStatus = LCD485WAITING;
					TxRxCount_LCD485 = 0;					
				}
				else if(TxRxCount_LCD485 == ReceiveDataNumber_ThisTime)
				{
					LCD485ComStatus = LCD485READYTRANSMIT;
					LCD485ReceiveCompletTime = HAL_GetTick();
				}
			}
		}
		return HAL_OK;
	}
	else
	{
		/* Clear RXNE interrupt flag */
		__HAL_UART_SEND_REQ(&hlpuart1, UART_RXDATA_FLUSH_REQUEST);
		return HAL_BUSY;
	}
}

static HAL_StatusTypeDef LCD485_Transmit_IT(void)
{
	/* Check that a Tx process is ongoing */
	if (hlpuart1.gState == HAL_UART_STATE_BUSY_TX)
	{
		if(LCD485ComStatus == LCD485TRANSMITING)
		{
			if(TxRxCount_LCD485 > TransitDataNumber_ThisTime)
			{
				/* Disable the UART Transmit Data Register Empty Interrupt */
				CLEAR_BIT(hlpuart1.Instance->CR1, USART_CR1_TXEIE);
				/* Enable the UART Transmit Complete Interrupt */
				SET_BIT(hlpuart1.Instance->CR1, USART_CR1_TCIE);
				TxRxCount_LCD485 = 0;
				LCD485ComStatus = LCD485WAITING;
				return HAL_OK;
			}
			else
			{
				hlpuart1.Instance->TDR = (uint8_t)(RTBuffer_LCD485[TxRxCount_LCD485]);
				TxRxCount_LCD485++;
				return HAL_OK;
			}
		}
		return HAL_OK;
	}
	else
	{
		return HAL_BUSY;
	}
}

void	LCD485_IRQHandler(void)
{
	uint32_t isrflags   = READ_REG(hlpuart1.Instance->ISR);
	uint32_t cr1its     = READ_REG(hlpuart1.Instance->CR1);
	uint32_t cr3its;
	uint32_t errorflags;

	/* If no error occurs */
	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
	if (errorflags == RESET)
	{
		/* UART in mode Receiver ---------------------------------------------------*/
		if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{
			LCD485_Receive_IT();
			return;
		}
	}  

	/* If some errors occur */
	cr3its = READ_REG(hlpuart1.Instance->CR3);
	if( (errorflags != RESET)&& ( ((cr3its & USART_CR3_EIE) != RESET)|| ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)) )
	{
		/* UART parity error interrupt occurred -------------------------------------*/
		if(((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_PEF);
			hlpuart1.ErrorCode |= HAL_UART_ERROR_PE;
		}

		/* UART frame error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_FEF);

			hlpuart1.ErrorCode |= HAL_UART_ERROR_FE;
		}

		/* UART noise error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_NEF);
			hlpuart1.ErrorCode |= HAL_UART_ERROR_NE;
		}
    
		/* UART Over-Run interrupt occurred -----------------------------------------*/
		if(((isrflags & USART_ISR_ORE) != RESET)&&(((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET)))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_OREF);
			hlpuart1.ErrorCode |= HAL_UART_ERROR_ORE;
		}

		/* Call UART Error Call back function if need be --------------------------*/
		if(hlpuart1.ErrorCode != HAL_UART_ERROR_NONE)
		{
			/* UART in mode Receiver ---------------------------------------------------*/
			if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
			{
				LCD485_Receive_IT();
			}
		}
		return;
	} /* End if some error occurs */

	/* UART wakeup from Stop mode interrupt occurred ---------------------------*/
	cr3its = READ_REG(hlpuart1.Instance->CR3);
	if(((isrflags & USART_ISR_WUF) != RESET) && ((cr3its & USART_CR3_WUFIE) != RESET))
	{
		__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_WUF);
		/* Set the UART state ready to be able to start again the process */
		hlpuart1.gState  = HAL_UART_STATE_READY;
		hlpuart1.RxState = HAL_UART_STATE_READY;
		HAL_UARTEx_WakeupCallback(&hlpuart1);
		return;
	}

	/* UART in mode Transmitter ------------------------------------------------*/
	if(((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
	{
		LCD485_Transmit_IT();
		return;
	}

	/* UART in mode Transmitter (transmission end) -----------------------------*/
	if(((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
//		UART_EndTransmit_IT(&hlpuart1);
		/* Disable the UART Transmit Complete Interrupt */
		CLEAR_BIT(hlpuart1.Instance->CR1, USART_CR1_TCIE);
		/* Tx process is ended, restore huart->gState to Ready */
		hlpuart1.gState = HAL_UART_STATE_READY;
		HAL_UART_TxCpltCallback(&hlpuart1);
		EnableLCD485Receive();
		return;
	}
}

void	AutoTransmit(float TempDistance)
{
	uint16_t TempCRCCode;
	uint8_t TempC;
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;
	if(AutoTransmit_Flag != 0xA5)
	{
		AutoTransmit_Flag = 0xA5;
		if(GetComAddress_485() == 0)
		{
			RTBuffer_LCD485[0] = 0xAA;
		}
		else
		{
			RTBuffer_LCD485[0] = GetComAddress_485();
		}
		RTBuffer_LCD485[1] = 3;
		RTBuffer_LCD485[2] = 4;
		TempFloatToChar.TempF = TempDistance;
		for(TempC = 3; TempC < 7; TempC++)
		{
			RTBuffer_LCD485[TempC] = TempFloatToChar.TempChar[TempC-3];
		}
		TransitDataNumber_ThisTime = RTBuffer_LCD485[2]+3;
	
		TempCRCCode = crc_check(TransitDataNumber_ThisTime);
		RTBuffer_LCD485[TransitDataNumber_ThisTime] = TempCRCCode;
		TransitDataNumber_ThisTime++;
		RTBuffer_LCD485[TransitDataNumber_ThisTime] = TempCRCCode>>8;
		TransitDataNumber_ThisTime++;
		LCD485ComStatus = LCD485TRANSMITING;
		TxRxCount_LCD485 = 0;
		HAL_UART_Transmit_IT(&hlpuart1, (uint8_t *)RTBuffer_LCD485, TransitDataNumber_ThisTime);
	}
}
