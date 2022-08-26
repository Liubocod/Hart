#include	"main.h"
#include	"usart.h"
#include	"ComLCDHandle.h"
#include 	"DistanceDataHandle.h"
#include	"EchoHandle.h"
#include	"Eeprom.h"
#include	"IWR1443.h"
#include	"BlueTooth_Com.h"
#include	"perf_management.h"

uint8_t RTBuffer_BT[MAX_BT_BUFFER];
uint8_t TxRxCount_BT;
uint8_t BTTransitDataNumber_ThisTime,BTReceiveDataNumber_ThisTime;
uint8_t BTComStatus;
uint8_t SetBLTooth_Adv_Data_Flag = 0;
uint16_t BTStartAddress;
uint16_t BLToothWorkCount;
uint32_t BTReceiveCompletTime;
uint16_t BLToothStatusCount_L,BLToothStatusCount_H;
uint16_t BLToothStatusTime_L,BLToothStatusTime_H;
uint8_t BLToothStatusInf = 0xFF;
uint16_t ProductType_Int,ProductSL_Int;
uint8_t Uart_Adv_Data[5];
uint8_t ReceiveCount_ST;
/*--------------------------MODBUS CRC计算----------------------------------------------*/
uint16_t	BT_crc_check(uint8_t Start_Byte,uint8_t pack_byte)
{
	uint16_t polyn_m = 0xA001;
	uint8_t crc_i,crc_j;
	uint16_t crc = 0xFFFF;
	for(crc_i = Start_Byte;crc_i < pack_byte;crc_i++)
	{
		crc ^= RTBuffer_BT[crc_i];
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

void	Reset_Uart_Adv_Data(void)
{
	ReceiveCount_ST = 0;
	Uart_Adv_Data[0] = 0;
	Uart_Adv_Data[1] = 0;
	Uart_Adv_Data[2] = 0;
	Uart_Adv_Data[3] = 0;
}

void	HandleStatus_BLT(uint8_t TempUartD)
{
	Uart_Adv_Data[ReceiveCount_ST] = TempUartD;
	if(Uart_Adv_Data[0] == 0xA5)
	{
		ReceiveCount_ST++;
		if(Uart_Adv_Data[1] == 0x5A)
		{
			if(Uart_Adv_Data[2] == 0x5A)
			{
				if(Uart_Adv_Data[3] == 0xAA)
				{
					BLToothStatusInf = POWERON_BLTOOTH;
					BLToothStatusCount_H = 0;
					BLToothStatusCount_L = 0;
					Reset_Uart_Adv_Data();
				}
				else if(Uart_Adv_Data[3] == 0x55)
				{
					BLToothStatusInf = DISCONNECTED_BLTOOTH;
					SetBLTooth_Adv_Data_Flag = 1;
					BLToothStatusCount_H = 0;
					BLToothStatusCount_L = 0;
					Reset_Uart_Adv_Data();
				}
			}
			else if(ReceiveCount_ST >= 3)
			{
				Reset_Uart_Adv_Data();
			}
		}
		else if(ReceiveCount_ST >= 2)
		{
			Reset_Uart_Adv_Data();
		}
	}
	else
	{
		Reset_Uart_Adv_Data();
	}
}

void	SetProductType_Int(uint16_t Temp_ProductType)
{
	ProductType_Int = Temp_ProductType;
}

void	SetProductSL_Int(uint16_t Temp_ProductSL)
{
	ProductSL_Int = Temp_ProductSL;
}

void	Set_BLTooth_Adv_Data(uint8_t CurrenStatus)
{
	SetBLTooth_Adv_Data_Flag = CurrenStatus;
}

void	ReadBLTooth_Status(void)
{
	if(HAL_GPIO_ReadPin(BT_STATUS_GPIO_Port,BT_STATUS_Pin) == GPIO_PIN_RESET)
	{
		BLToothStatusCount_L++;
		if(BLToothStatusCount_L > BLTOOTH_STATUS_MAXTIME)
		{
			BLToothStatusCount_L = BLTOOTH_STATUS_MAXTIME;
		}
		else if(BLToothStatusCount_L > BLTOOTH_STATUS_L_MAXTIME)
		{
//			if(SetBLTooth_Adv_Data_Flag == 0)
//			{
				SetBLTooth_Adv_Data_Flag = 1;
//			}
		}
		if((BLToothStatusInf == POWERON_BLTOOTH)&&(SetBLTooth_Adv_Data_Flag == 0))
		{
			BLToothStatusInf = SETADV_DATA_BLTOOTH;
		}
		
		BLToothStatusTime_H = BLToothStatusCount_H;
		BLToothStatusCount_H = 0;
	}
	else
	{
		BLToothStatusCount_H++;
		if(BLToothStatusCount_H > BLTOOTH_STATUS_MAXTIME)
		{
			BLToothStatusCount_H = BLTOOTH_STATUS_MAXTIME;
//			if(BLToothStatusInf == CONNECTED_BLTOOTH)
				SetBLTooth_Adv_Data_Flag = 1;
//			BLToothStatusCount_H = 0;
		}
		else if(BLToothStatusInf == SETADV_DATA_BLTOOTH)
		{
			BLToothStatusInf = CONNECTED_BLTOOTH;
		}
		
		BLToothStatusTime_L = BLToothStatusCount_L;
		BLToothStatusCount_L = 0;
	}
}

void	RecordBTInf(void)
{
	RecordRealPerfToFRAM((float)(BLToothStatusInf),90);
	RecordRealPerfToFRAM((float)(SetBLTooth_Adv_Data_Flag),91);
//	RecordRealPerfToFRAM((float)(BLToothStatusCount_H),93);
//	RecordRealPerfToFRAM((float)(BLToothStatusCount_L),94);
}

void	Set_BLT_Adv_Data(void)
{
	uint8_t TempC;
	uint16_t TempCRC;
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;

	if((SetBLTooth_Adv_Data_Flag == 3)&&(BLToothStatusInf == POWERON_BLTOOTH))
	{
		RTBuffer_BT[0] = 0xA5;
		RTBuffer_BT[1] = 0x5A;
		RTBuffer_BT[2] = 0x5A;
		RTBuffer_BT[3] = 0xAA;
		RTBuffer_BT[4] = TAG_LENGTH+19;
		for(TempC = 0; TempC < TAG_LENGTH; TempC++)
		{
			RTBuffer_BT[5+TempC] = *(GetPara_Char(21)+TempC);
		}
		RTBuffer_BT[5+TAG_LENGTH] = *GetPara_Char(30);
		RTBuffer_BT[6+TAG_LENGTH] = 0xFF-RTBuffer_BT[5+TAG_LENGTH];
		
		RTBuffer_BT[7+TAG_LENGTH] = ProductType_Int;
		RTBuffer_BT[8+TAG_LENGTH] = ProductType_Int>>8;
		
		RTBuffer_BT[9+TAG_LENGTH] = ProductSL_Int;
		RTBuffer_BT[10+TAG_LENGTH] = ProductSL_Int>>8;
		
		RTBuffer_BT[11+TAG_LENGTH] = *GetPara_Char(5);
		
		GetPara_Float(&RTBuffer_BT[12+TAG_LENGTH],1);

		GetPara_Float(&RTBuffer_BT[16+TAG_LENGTH],0);

		TempFloatToChar.TempF = HartReadMasterVariable(1);
		for(TempC = 0; TempC < 4; TempC++)
		{
			RTBuffer_BT[20+TAG_LENGTH+TempC] = TempFloatToChar.TempChar[TempC];
		}
		
		TempCRC = BT_crc_check(5,RTBuffer_BT[4]+5);
		RTBuffer_BT[RTBuffer_BT[4]+5] = TempCRC;
		RTBuffer_BT[RTBuffer_BT[4]+6] = TempCRC>>8;
		TxRxCount_BT = 0;
		BTComStatus = BTTRANSMITING;
		BTTransitDataNumber_ThisTime = RTBuffer_BT[4]+7;
		HAL_UART_Transmit_IT(&huart4, (uint8_t *)RTBuffer_BT, BTTransitDataNumber_ThisTime);
		BLToothStatusCount_H = 0;
		BLToothStatusCount_L = 0;
		SetBLTooth_Adv_Data_Flag = 0;
	}
	else
	{
		if(SetBLTooth_Adv_Data_Flag == 1)
		{
			if((BLToothStatusCount_H > BLTOOTH_STATUS_H_MAXTIME)||(BLToothStatusCount_L > BLTOOTH_STATUS_H_MAXTIME))
			{
			ENABLE_RESET_BLTOOTH;
			BLToothStatusCount_L = 0;
			BLToothStatusCount_H = 0;
			Reset_Uart_Adv_Data();
			BLToothStatusInf = 0xFF;
			SetBLTooth_Adv_Data_Flag = 2;
			}
		}
		else if(SetBLTooth_Adv_Data_Flag == 2)
		{
			if((BLToothStatusCount_H > 10)||(BLToothStatusCount_L > 10))
			{
				MX_UART4_Init();
				EnableBTReceive();
				DISABLE_RESET_BLTOOTH;
				SetBLTooth_Adv_Data_Flag = 3;
			}
		}
	}
	if(BLToothStatusCount_H > BLTOOTH_STATUS_H_MAXTIME)
	{
		if((BLToothStatusInf == 0xFF)&&(SetBLTooth_Adv_Data_Flag == 3))
		{
//			BLToothStatusInf = POWERON_BLTOOTH;
			HAL_UART_DeInit(&huart4);
			SetBLTooth_Adv_Data_Flag = 1;
			BLToothStatusCount_H = 0;
//			RecordPerfToFRAM(91);
		}
	}
	/*else if((BLToothStatusInf == POWERON_BLTOOTH)&&(SetBLTooth_Adv_Data_Flag == 0)&&(BLToothStatusCount_H > BLTOOTH_STATUS_H_MAXTIME))
	{
		HAL_UART_DeInit(&huart4);
		SetBLTooth_Adv_Data_Flag = 1;
		BLToothStatusCount_H = 0;
		RecordPerfToFRAM(90);
	}*/
}

void	EnableBTReceive(void)
{
	DIR_485B_RX;
	HAL_UART_Receive_IT(&huart4, (uint8_t *)RTBuffer_BT, 50);
}

void	SendMasterVariable_485(void)
{
	uint8_t TempC,TempD;
	uint16_t TempI;
	uint16_t TempRegisterAddress;
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;

	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
	TempI = RTBuffer_BT[5]+BTStartAddress;
	TempD = 0;
	for(TempRegisterAddress = BTStartAddress; TempRegisterAddress < TempI; TempRegisterAddress = TempRegisterAddress+2)
	{//上位机读数据
		TempFloatToChar.TempF = HartReadMasterVariable((TempRegisterAddress-500)/2);
		for(TempC = 0; TempC < 4; TempC++)
		{
			RTBuffer_BT[4*TempD+TempC+3] = TempFloatToChar.TempChar[TempC];
		}
		TempD++;
	}
}

/*485通信，发送回波曲线数据、距离测量值（浮点数）、频率、物料回波位置、物料回波信噪比、物料回波幅度、物料回波分辨率*/
void	BT_SendCurveInformation(void)
{
	uint8_t	TempRegisterAddress;
	uint8_t TempC;
	union
	{
		float TempF;
		uint8_t TempChar[4];
	}TempFloatToChar;
//	for(TempC = 3; TempC < 103; TempC++)
//		RTBuffer_BT[TempC] = LcdEcho[TempC-3];
//	ReadCurveData(&RTBuffer_BT[3]);
	for(TempC = 3; TempC < 103; TempC++)
	{
		RTBuffer_BT[TempC] = LcdEcho[TempC-3];
	}
	TempFloatToChar.TempF = HartReadMasterVariable(1);
	for(TempC = 103; TempC < 107; TempC++)
	{
		RTBuffer_BT[TempC] = TempFloatToChar.TempChar[TempC-103];
	}
	TempFloatToChar.TempF = GetBestEchoAmplitude();

	for(TempC = 107; TempC < 111; TempC++)
	{
		RTBuffer_BT[TempC] = TempFloatToChar.TempChar[TempC-107];
	}
	RTBuffer_BT[111] = GetMarkBestWavePostion();
	RTBuffer_BT[112] = 0;//(unsigned char)(SignalNoiseRadio);
	RTBuffer_BT[113] = (uint8_t)(GetBestEchoAmplitude()/AMPLITUDE_COV_SAMPLE_255);
	RTBuffer_BT[114] = (uint8_t)(GetBestEchoDistinctiveness());

	TempRegisterAddress = (uint8_t)((BTStartAddress-800)*2);
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
	for(TempC = 3; TempC < BTTransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_BT[TempC] = RTBuffer_BT[TempC+TempRegisterAddress];
	}
	SetCurveStart_EndPoint(0,0);
}

void	BT_SendPara_Char(void)
{
	uint8_t TempC;
	uint16_t TempRegisterAddress;
	RTBuffer_BT[3] = 0;
	RTBuffer_BT[4] = *GetPara_Char(5);
	for(TempC = 1; TempC < 5; TempC++)
	{
		RTBuffer_BT[4+TempC*2] = *GetPara_Char(TempC-1);
		RTBuffer_BT[3+TempC*2] = 0;
	}
	for(TempC = 7; TempC < 17; TempC++)
	{
		RTBuffer_BT[4+(TempC-2)*2] = *GetPara_Char(TempC);
		RTBuffer_BT[3+(TempC-2)*2] = 0;
	}
	RTBuffer_BT[34] = GetAllUserPara_HandleMethod();	
	RTBuffer_BT[33] = 0;	
	for(TempC = 0; TempC < DATE_LENGTH; TempC++)
	{/*时间*/
		RTBuffer_BT[35+TempC] = *(GetPara_Char(18)+TempC);
	}
	for(TempC = 0; TempC < SENSOR_SN_LENGTH; TempC++)
	{
		RTBuffer_BT[43+TempC] = *(GetPara_Char(19)+TempC);
	}
	for(TempC = 0; TempC < DESCRIPTOR_LENGTH; TempC++)
	{
		RTBuffer_BT[53+TempC] = *(GetPara_Char(20)+TempC);
	}
	RTBuffer_BT[69] = VERSION0;
	RTBuffer_BT[70] = VERSION1;
	RTBuffer_BT[71] = VERSION2;
	RTBuffer_BT[72] = 0;
	RTBuffer_BT[73] = 0;
	RTBuffer_BT[74] = *GetPara_Char(25);
	RTBuffer_BT[75] = 0;
	RTBuffer_BT[76] = *GetPara_Char(23);
	RTBuffer_BT[77] = 0;
	RTBuffer_BT[78] = *GetPara_Char(24);
	
	RTBuffer_BT[79] = (uint8_t)(Get_IWR1443_Version());
	RTBuffer_BT[80] = (uint8_t)(Get_IWR1443_Version()>>8);
	RTBuffer_BT[81] = *GetPara_Char(28);
	RTBuffer_BT[82] = *GetPara_Char(26);
	RTBuffer_BT[83] = *GetPara_Char(29);
	RTBuffer_BT[84] = *GetPara_Char(27);
	
	TempRegisterAddress = (uint8_t)((BTStartAddress-9)*2);
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
	for(TempC = 3; TempC < BTTransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_BT[TempC] = RTBuffer_BT[TempC+TempRegisterAddress];
	}
}

void	BT_SendMeter_TAG(void)
{
	uint8_t TempC;
//	uint16_t TempRegisterAddress;
	for(TempC = 0; TempC < DESCRIPTOR_LENGTH; TempC++)
	{
		RTBuffer_BT[3+TempC] = *(GetPara_Char(21)+TempC);
	}
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
}

void	BT_SendExpandPara_Char(void)
{
	uint8_t TempC;
	uint16_t TempRegisterAddress;
	for(TempC = 0; TempC < EXPANDCHARPARANUMBERS; TempC++)
	{
		RTBuffer_BT[4+TempC*2] = *GetPara_Char(EXPANDCHARPARA_STARTADDRESS_485+TempC);
		RTBuffer_BT[3+TempC*2] = 0;
	}
	
	TempRegisterAddress = (uint8_t)((BTStartAddress-0x1000)*2);
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
	for(TempC = 3; TempC < BTTransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_BT[TempC] = RTBuffer_BT[TempC+TempRegisterAddress];
	}
}

void	BT_SendPerfData(void)
{
	uint8_t TempParaNumber,TempC;
	uint16_t TempRegisterAddress;
	TempParaNumber = (uint8_t)((BTStartAddress-1000)/2);
	TempRegisterAddress = RTBuffer_BT[5]/2;
	
	for(TempC = 0; TempC < TempRegisterAddress; TempC++)
	{
		PerfDataRead(TempParaNumber,(float *)(&RTBuffer_BT[3+TempC*4]));
		TempParaNumber++;
	}
	
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
}

void	BT_SendPara_Float(void)
{
	uint8_t TempParaNumber,TempC;
	uint16_t TempRegisterAddress;
	for(TempParaNumber = 0; TempParaNumber < 9; TempParaNumber++)
	{
		GetPara_Float(&RTBuffer_BT[3+TempParaNumber*4],TempParaNumber);
	}
	
	for(TempParaNumber = 10; TempParaNumber < 21; TempParaNumber++)
	{
		GetPara_Float(&RTBuffer_BT[3+TempParaNumber*4],TempParaNumber);
	}

	GetPara_Float(&RTBuffer_BT[87],9);
	
	TempRegisterAddress = (BTStartAddress-50)*2;
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
	for(TempC = 3; TempC < BTTransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_BT[TempC] = RTBuffer_BT[TempC+TempRegisterAddress];
	}
}

void	BT_SendExpandPara_Float(void)
{
	uint8_t TempC;
	uint16_t TempI;
	uint16_t TempRegisterAddress;
	
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
	TempRegisterAddress = (BTStartAddress-0x2000)/2;
	TempI = RTBuffer_BT[5]/2+TempRegisterAddress;
	TempC = 0;
	for(; TempRegisterAddress < TempI; TempRegisterAddress++)
	{//上位机读数据
		GetPara_Float(&RTBuffer_BT[3+TempC*4],EXPANDFLOATPARA_STARTADDRESS_485+TempRegisterAddress);
		TempC++;
	}
}

/*485通信，根据接收到上位机的命令帧，保存单字节参数*/
void	BT_SavePara_Char(void)
{
	if(BTStartAddress < 14)
	{
		if(BTStartAddress == 9)
		{
			SavePara_CharType(MODIFYPARAMODE_BLUETL,5,RTBuffer_BT[8]);
		}
		else if(BTStartAddress >= 10)
		{
			SavePara_CharType(MODIFYPARAMODE_BLUETL,BTStartAddress-10,RTBuffer_BT[8]);
		}
	}
	else if((BTStartAddress >= 14)&&(BTStartAddress <= 46))
	{
		if(BTStartAddress != 24)
			BTStartAddress = BTStartAddress+3;
		switch(BTStartAddress)
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
				SavePara_CharType(MODIFYPARAMODE_BLUETL,BTStartAddress-10,RTBuffer_BT[8]);
			break;
			case 27:
				SetAllUserPara_HandleMethod(MODIFYPARAMODE_BLUETL,RTBuffer_BT[8]);
			break;
			case 28:
				SavePara_StringType(BTStartAddress-10,&RTBuffer_BT[7]);
			break;
			case 32:
				SavePara_StringType(BTStartAddress-13,&RTBuffer_BT[7]);
			break;
			case 37:
				SavePara_StringType(BTStartAddress-17,&RTBuffer_BT[7]);
			break;
			case 47:
				Save_Eeprom_EnablePara_Protect(&RTBuffer_BT[7]);
			break;
			case 48:
				SavePara_CharType(MODIFYPARAMODE_BLUETL,BTStartAddress-25,RTBuffer_BT[8]);
			break;
			case 49:
				SavePara_CharType(MODIFYPARAMODE_BLUETL,BTStartAddress-25,RTBuffer_BT[8]);
			break;
			default:break;
		}
	}
	else if(BTStartAddress > 46)
	{
		switch(BTStartAddress)
		{
			case 48:
				SavePara_CharType(MODIFYPARAMODE_BLUETL,BTStartAddress-22,RTBuffer_BT[8]);
			break;
			case 49:
//				SavePara_StringType(BTStartAddress-22,&RTBuffer_BT[7]);
				SavePara_CharType(MODIFYPARAMODE_BLUETL,BTStartAddress-22,RTBuffer_BT[8]);
			break;
			default:break;
		}
	}
}

/*485通信，根据接收到上位机的命令帧，保存4字节浮点数参数*/
void	BT_SavePara_Float(void)
{
	switch(BTStartAddress)
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
			SavePara_FloatType(MODIFYPARAMODE_BLUETL,(BTStartAddress-50)/2, (float*)(&RTBuffer_BT[7]));
		break;
		case 58:
			SavePara_FloatType(MODIFYPARAMODE_BLUETL,4,(float*)(&RTBuffer_BT[7]));
			
			if(RTBuffer_BT[2] >= 16)
			{
				SavePara_FloatType(MODIFYPARAMODE_BLUETL,5, (float*)(&RTBuffer_BT[11]));
			}
		break;
		case 68:
			if(*(float*)(&RTBuffer_BT[7]) == 1)
			{
				ModifyCurrent(MODIFYPARAMODE_BLUETL);
				*(float*)(&RTBuffer_BT[7]) = 18;
			}
			if((*(float*)(&RTBuffer_BT[7]) < MINCURRENT_SET)||(*(float*)(&RTBuffer_BT[7]) > MAXCURRENT_SET))
				*(float*)(&RTBuffer_BT[7]) = 0;
			SetOutCurrentVal(*(float*)(&RTBuffer_BT[7]));
		break;
		case 70:
			Set_PWM4mABias_Value(*(float*)(&RTBuffer_BT[7]));
		break;
		case 72:
			Set_PWM18mABias_Value(*(float*)(&RTBuffer_BT[7]));
		break;
		case 92:
			SavePara_FloatType(MODIFYPARAMODE_BLUETL,9, (float*)(&RTBuffer_BT[7]));
		break;
		default:break;
	}
}

void	BT_HandleLowAdressRegister_10(void)
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
	TempInt[6] = (uint16_t)(HartReadMasterVariable(2)*100);
	TempInt[7] = (uint16_t)(HartReadMasterVariable(3)*100);
	for(TempC = 0; TempC < 8; TempC++)
	{
		RTBuffer_BT[3+2*TempC] = TempInt[TempC]>>8;
		RTBuffer_BT[4+2*TempC] = TempInt[TempC];
	}
	RTBuffer_BT[19] = ReadMeterWorkStatus();
	
	BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
	for(TempC = 3; TempC < BTTransitDataNumber_ThisTime; TempC++)
	{
		RTBuffer_BT[TempC] = RTBuffer_BT[TempC+BTStartAddress*2];
	}
}

uint8_t	GetBlueToothStatus(void)
{
	return(BLToothStatusInf);
}

void	BT_ModbusProtocol(void)
{
	uint16_t TempCRCCode;
	float TempF[3];
//	if(*GetPara_Char(23) == 0)
//	{
	if((BTComStatus == BTREADYTRANSMIT)&&((HAL_GetTick()-BTReceiveCompletTime) > INTERVALTIME_MAX)&&(Get_IWR1443_Status() == IWR1443_WAIT_STATUS))
	{
		BLToothStatusCount_H = 0;
		BLToothStatusCount_L = 0;
		BLToothStatusInf = CONNECTED_BLTOOTH;
		BLToothWorkCount++;
		if(BLToothWorkCount > MAXRESETPERF_COUNT)
		{
			ResetPerfData(BLTOOTHERRORTIMES_TH);
			BLToothWorkCount = 0;
		}
		RecordPerfToFRAM(BLTOOTHTIMES_TOTAL);
		TempCRCCode = RTBuffer_BT[TxRxCount_BT-1]+RTBuffer_BT[TxRxCount_BT]*256;
		if(TempCRCCode == BT_crc_check(0,TxRxCount_BT-1))/*判断CRC校验码对不对*/
		{
			DIR_485B_TX;
			BTStartAddress = RTBuffer_BT[2];
			BTStartAddress <<= 8;
			BTStartAddress += RTBuffer_BT[3];
			if(RTBuffer_BT[MODBUSCOMMAND_INDEX] != 0x10)
				RTBuffer_BT[2] = RTBuffer_BT[5]*2;
			
			switch(RTBuffer_BT[MODBUSCOMMAND_INDEX])
			{
				case	0x01:
					break;
				case	0x02:
					break;
				case	0x03:/*上位机读仪表参数命令*/
					if(BTStartAddress < 9)
					{
						BT_HandleLowAdressRegister_10();
					}
					else if(BTStartAddress < 50)
					{
						BT_SendPara_Char();
					}
					else if(BTStartAddress < 150)
					{
						BT_SendPara_Float();
					}
					else if((BTStartAddress >= 500)&&(BTStartAddress <= 508))
					{
						SendMasterVariable_485();
					}
					else if(BTStartAddress == 600)
					{
						SendReadFakeCurveInfo(&RTBuffer_BT[3]);
						BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
					}
					else if((BTStartAddress >= 750)&&(BTStartAddress < 800))
					{
						SendFakeCurveInfo(&RTBuffer_BT[3]);
						BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
					}
					else if((BTStartAddress >= 800)&&(BTStartAddress < 1000))
					{
						BT_SendCurveInformation();
					}
					else if((BTStartAddress >= 1000)&&(BTStartAddress < 2000))
					{
						BT_SendPerfData();
					}
					else if((BTStartAddress >= 2000)&&(BTStartAddress < 2250))
					{
						ReadModifyParaInfo(&RTBuffer_BT[3],(BTStartAddress-2000)/10);
						BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
					}
					else if((BTStartAddress >= 2300)&&(BTStartAddress < 3500))
					{
						ReadJumpCurveInfo_485(&RTBuffer_BT[3],(BTStartAddress-2300),RTBuffer_BT[2]);
						BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
					}
					else if(BTStartAddress == 3599)
					{
						RTBuffer_BT[3] = 0;
						RTBuffer_BT[4] = GetPara_PointsNum();
						BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
					}
					else if((BTStartAddress >= 3600)&&(BTStartAddress < 4000))
					{//读线性参数值
						ReadParaL_LCD((BTStartAddress-3600)/4+55,(float*)(&RTBuffer_BT[3]));
						BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
					}
					else if((BTStartAddress >= 0x1000)&&(BTStartAddress < 0x2000))
					{
						BT_SendExpandPara_Char();
					}
					else if((BTStartAddress >= 0x2000)&&(BTStartAddress < 0x3000))
					{
						BT_SendExpandPara_Float();
					}
					else if(BTStartAddress == 0x3000)
					{
						BT_SendMeter_TAG();
					}
					else if(BTStartAddress >= 0x3500)
					{
						Get_MulEcho_Information(RTBuffer_BT[2]/8,&RTBuffer_BT[3]);
						BTTransitDataNumber_ThisTime = RTBuffer_BT[2]+3;
					}
					break;
				case	0x10:
					RecordPerfToFRAM(MODFYPARATIMES_BLTOOTH);
					if(BTStartAddress < 9)
					{
					}
					else if(BTStartAddress <= 49)
					{
						BT_SavePara_Char();
					}
					else if(BTStartAddress < 150)
					{
						BT_SavePara_Float();
					}
					else if(BTStartAddress == 600)
					{/*曲线操作*/
						TempF[0] = *(float*)(&RTBuffer_BT[7]);
						TempF[1] = *(float*)(&RTBuffer_BT[11]);
						TempF[2] = *(float*)(&RTBuffer_BT[15]);
						CreatFakeCurve(&TempF[0]);						
//						SetCurveStart_EndPoint(*((float*)(&RTBuffer_BT[7])),*((float*)(&RTBuffer_BT[11])));
					}
					else if(BTStartAddress == 606)
					{
						SetCurveStart_EndPoint(*((float*)(&RTBuffer_BT[7])),*((float*)(&RTBuffer_BT[11])));
					}
					else if(BTStartAddress == 1000)
					{
						ClearAllPerf();
					}
					else if((BTStartAddress >= 2000)&&(BTStartAddress < 2250))
					{
						
					}
					else if((BTStartAddress >= 2300)&&(BTStartAddress < 3500))
					{
//						SetJumpEchoInfoP(0);
						ClearJumpCurveInfo();
					}
					else if(BTStartAddress == 3599)
					{
						SavePara_PointsNum(RTBuffer_BT[8]);
					}
					else if((BTStartAddress >= 3600)&&(BTStartAddress < 4000))
					{//修改线性参数值
						SavePara_LineType((BTStartAddress-3600)/4+55, (float*)(&RTBuffer_BT[7]));
					}
					else if((BTStartAddress >= 0x1000)&&(BTStartAddress < 0x2000))
					{
						SavePara_CharType(MODIFYPARAMODE_BLUETL,BTStartAddress-0x1000+EXPANDCHARPARA_STARTADDRESS_485,RTBuffer_BT[8]);
					}
					else if((BTStartAddress >= 0x2000)&&(BTStartAddress < 0x3000))
					{
						SavePara_FloatType(MODIFYPARAMODE_BLUETL,(BTStartAddress-0x2000)/2+EXPANDFLOATPARA_STARTADDRESS_485, (float*)(&RTBuffer_BT[7]));
					}
					else if(BTStartAddress == 0x3000)
					{
						SavePara_StringType(21,&RTBuffer_BT[7]);
					}
					BTTransitDataNumber_ThisTime = 6;
					break;
					default:break;
			}
			TempCRCCode = BT_crc_check(0,BTTransitDataNumber_ThisTime);
			RTBuffer_BT[BTTransitDataNumber_ThisTime] = TempCRCCode;
			BTTransitDataNumber_ThisTime++;
			RTBuffer_BT[BTTransitDataNumber_ThisTime] = TempCRCCode>>8;
			BTTransitDataNumber_ThisTime++;
			BTComStatus = BTTRANSMITING;
			TxRxCount_BT = 0;
			HAL_UART_Transmit_IT(&huart4, (uint8_t *)RTBuffer_BT, BTTransitDataNumber_ThisTime);
		}
		else
		{/*CRC出错*/
			BTComStatus = BTWAITING;
			RecordPerfToFRAM(BLTOOTHERRORTIMES);
			RecordPerfToFRAM(BLTOOTHERRORTIMES_TH);
			EnableBTReceive();
		}
	}
//	}
}

static	HAL_StatusTypeDef BT_Receive_IT(void)
{
//	uint8_t  TempC;
	uint16_t  uhdata;

	/* Check that a Rx process is ongoing */
	if(huart4.RxState == HAL_UART_STATE_BUSY_RX)
	{
		uhdata = (uint16_t) READ_REG(huart4.Instance->RDR)&0x00FF;
		HandleStatus_BLT(uhdata);
		if(BTComStatus == BTWAITING)
		{
			if((uhdata == GetComAddress())||(uhdata == 0xAA))
			{
				BTComStatus = BTRECEIVING;
				RTBuffer_BT[DEVICEADDRESS_INDEX] = uhdata;
				TxRxCount_BT = 0;
			}
//			else
//			{
//			}
		}
		else if(BTComStatus == BTRECEIVING)
		{
			TxRxCount_BT++;
			RTBuffer_BT[TxRxCount_BT] = uhdata;
			if(TxRxCount_BT < 7)
			{
				if((RTBuffer_BT[MODBUSCOMMAND_INDEX] <= 0x05)&&(RTBuffer_BT[MODBUSCOMMAND_INDEX] >= 0x01))
				{
					BTReceiveDataNumber_ThisTime = 7;
				}
				else
				{
					if(RTBuffer_BT[MODBUSCOMMAND_INDEX] == 0x10)   //person computer write
					{
						if(TxRxCount_BT == 6)
						{
							BTReceiveDataNumber_ThisTime = RTBuffer_BT[6]+8;
							if(BTReceiveDataNumber_ThisTime > MAX_BT_BUFFER)
							{
								BTComStatus = BTWAITING;
								TxRxCount_BT = 0;
							}
						}
					}
					else
					{
						BTComStatus = BTWAITING;
						TxRxCount_BT = 0;
					}
				}
			}
			else
			{
				if(TxRxCount_BT >= MAX_BT_BUFFER)
				{
					BTComStatus = BTWAITING;
					TxRxCount_BT = 0;					
				}
				else if(TxRxCount_BT == BTReceiveDataNumber_ThisTime)
				{
					BTComStatus = BTREADYTRANSMIT;
					BTReceiveCompletTime = HAL_GetTick();
				}
			}
		}
		return HAL_OK;
	}
	else
	{
		/* Clear RXNE interrupt flag */
		__HAL_UART_SEND_REQ(&huart4, UART_RXDATA_FLUSH_REQUEST);
		return HAL_BUSY;
	}
}

static HAL_StatusTypeDef BT_Transmit_IT(void)
{
	/* Check that a Tx process is ongoing */
	if (huart4.gState == HAL_UART_STATE_BUSY_TX)
	{
		if(BTComStatus == BTTRANSMITING)
		{
			if(TxRxCount_BT > BTTransitDataNumber_ThisTime)
			{
				/* Disable the UART Transmit Data Register Empty Interrupt */
				CLEAR_BIT(huart4.Instance->CR1, USART_CR1_TXEIE);
				/* Enable the UART Transmit Complete Interrupt */
				SET_BIT(huart4.Instance->CR1, USART_CR1_TCIE);
				TxRxCount_BT = 0;
				BTComStatus = BTWAITING;
				return HAL_OK;
			}
			else
			{
				huart4.Instance->TDR = (uint8_t)(RTBuffer_BT[TxRxCount_BT]);
				TxRxCount_BT++;
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

void	BT_IRQHandler(void)
{
	uint32_t isrflags   = READ_REG(huart4.Instance->ISR);
	uint32_t cr1its     = READ_REG(huart4.Instance->CR1);
	uint32_t cr3its;
	uint32_t errorflags;

	/* If no error occurs */
	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
	if (errorflags == RESET)
	{
		/* UART in mode Receiver ---------------------------------------------------*/
		if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{
			BT_Receive_IT();
			return;
		}
	}  

	/* If some errors occur */
	cr3its = READ_REG(huart4.Instance->CR3);
	if( (errorflags != RESET)&& ( ((cr3its & USART_CR3_EIE) != RESET)|| ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)) )
	{
		/* UART parity error interrupt occurred -------------------------------------*/
		if(((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&huart4, UART_CLEAR_PEF);
			huart4.ErrorCode |= HAL_UART_ERROR_PE;
		}

		/* UART frame error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&huart4, UART_CLEAR_FEF);

			huart4.ErrorCode |= HAL_UART_ERROR_FE;
		}

		/* UART noise error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&huart4, UART_CLEAR_NEF);
			huart4.ErrorCode |= HAL_UART_ERROR_NE;
		}
    
		/* UART Over-Run interrupt occurred -----------------------------------------*/
		if(((isrflags & USART_ISR_ORE) != RESET)&&(((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET)))
		{
			__HAL_UART_CLEAR_IT(&huart4, UART_CLEAR_OREF);
			huart4.ErrorCode |= HAL_UART_ERROR_ORE;
		}

		/* Call UART Error Call back function if need be --------------------------*/
		if(huart4.ErrorCode != HAL_UART_ERROR_NONE)
		{
			/* UART in mode Receiver ---------------------------------------------------*/
			if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
			{
				BT_Receive_IT();
			}
		}
		return;
	} /* End if some error occurs */

	/* UART wakeup from Stop mode interrupt occurred ---------------------------*/
	cr3its = READ_REG(huart4.Instance->CR3);
	if(((isrflags & USART_ISR_WUF) != RESET) && ((cr3its & USART_CR3_WUFIE) != RESET))
	{
		__HAL_UART_CLEAR_IT(&huart4, UART_CLEAR_WUF);
		/* Set the UART state ready to be able to start again the process */
		huart4.gState  = HAL_UART_STATE_READY;
		huart4.RxState = HAL_UART_STATE_READY;
		HAL_UARTEx_WakeupCallback(&huart4);
		return;
	}

	/* UART in mode Transmitter ------------------------------------------------*/
	if(((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
	{
		BT_Transmit_IT();
		return;
	}

	/* UART in mode Transmitter (transmission end) -----------------------------*/
	if(((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
//		UART_EndTransmit_IT(&huart4);
		/* Disable the UART Transmit Complete Interrupt */
		CLEAR_BIT(huart4.Instance->CR1, USART_CR1_TCIE);
		/* Tx process is ended, restore huart->gState to Ready */
		huart4.gState = HAL_UART_STATE_READY;
		HAL_UART_TxCpltCallback(&huart4);
		EnableBTReceive();
		return;
	}
}
