#include "crc.h"
#include "i2c.h"
#include "usart.h"
#include "Eeprom.h"
#include "ComLCDHandle.h"
#include "EchoHandle.h"
#include "RFHandle.h"
#include "IWR1443.h"
#include "DistanceDataHandle.h"
#include "perf_management.h"
#include "CopyRightActivate.h"

//Private variables------------------------------------------------------------------------------------
LcdDataBuffer__TypeDef LcdDataBuffer;
uint8_t	Lcd_Read_Or_Write,LcdByteNumber;
uint16_t UpdateLCDCount,LcdCom_CRCErrorCount;
uint16_t IIcComWorkCount;
uint16_t CRCResult;
//-----------------------------------------------------------------------------------------------------
void	InitLcdData(void)
{
	uint8_t	TempC;
	ENABLE_LCDVCC;
	Lcd_Read_Or_Write = LCDWRITEDATA;
	for(TempC = 0; TempC < LCDBUFFERMAXLENGH; TempC++)
		LcdDataBuffer.LCDBufferC[TempC] = 0;
}
//-----------------------------------------------------------------------------------------------------
void	IncUpdateLCDCount(void)
{
	UpdateLCDCount++;
}

void	SartHandleLcdR(void)
{
	if(Lcd_Read_Or_Write == READLCD)
		UpdateLCDCount = UPDATELCDTIME;
}

void	LCD_ReadEeproPara(uint16_t ParaAddress, uint8_t ParaNumber)
{
	uint8_t TempC;
	uint8_t TempParaNumber;
	if(ParaAddress >= EXPANDFLOATPARA_STARTADDRESS_IIC)
	{
		for(TempParaNumber = 0; TempParaNumber < ParaNumber; TempParaNumber++)
		{
			GetPara_Float(&(LcdDataBuffer.LCDBufferC[4+TempParaNumber*4]),ParaAddress-EXPANDFLOATPARA_STARTADDRESS_IIC+TempParaNumber+FLOATEXPANDPARA_STARTA);
		}
	}
	else if(ParaAddress >= EXPANDCHARPARA_STARTADDRESS_IIC)
	{
		for(TempC = 0; TempC < ParaNumber; TempC++)
		{
			LcdDataBuffer.LCDBufferC[4+TempC] = *GetPara_Char(ParaAddress-EXPANDCHARPARA_STARTADDRESS_IIC+CHAREXPANDPARA_STARTA+TempC);
		}
	}
	else  if(ParaAddress >= LCD_FLOATPARA_STARTADDRESS_IIC)
	{
		for(TempParaNumber = 0; TempParaNumber < ParaNumber; TempParaNumber++)
		{
			GetPara_Float(&(LcdDataBuffer.LCDBufferC[4+TempParaNumber*4]),ParaAddress-LCD_FLOATPARA_STARTADDRESS_IIC+TempParaNumber);
		}
	}
	else if(ParaAddress >= LCD_CHARPARA_STARTADDRESS_IIC)
	{
		for(TempC = 0; TempC < ParaNumber; TempC++)
		{
			LcdDataBuffer.LCDBufferC[4+TempC] = *(GetPara_Char(ParaAddress-LCD_CHARPARA_STARTADDRESS_IIC)+TempC);
		}
	}
}

//------------------------------------------------------------------------------------------------------
void	LcdReadData(void)
{
	uint8_t TempC;
	switch(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS])
	{
		case	READ_UNIQUECODE_FUNCTION_INDEX:
			LcdDataBuffer.LCDBufferC[3] = DATA_BYTE_NUM_13;
			LcdDataBuffer.LCDBufferC[4] = PACKAGE_HEAD;
			LcdDataBuffer.LCDBufferC[11] = PACKAGE_TAIL;
				
		 	if(ReadUniqueCode(G_UniqueCode) == FALSE)
			{
				for (TempC = 0; TempC < UNIQUE_CODE_BYTE_NUM; TempC++)
				{
					LcdDataBuffer.LCDBufferC[5 + TempC] = 0;
				}
//				EventReport(EVENT_MCU_SOURCE, EVENT_ENCRYPT_ERROR_TYPE, EVENT_ENCRYPT_READ_ERR, I2C_FILE,
//				MASTER_HANDLE_DATA, EVENT_DANGEROUS_LEVEL8, G_SavedRunningTime); // 报告读取唯一码失败事件
			}
			else
			{
				for (TempC = 0; TempC < UNIQUE_CODE_BYTE_NUM; TempC++)
				{
					LcdDataBuffer.LCDBufferC[5 + TempC] = G_UniqueCode[TempC];
				}
			}
			break;	
		case	ACTIVATECODE_GENERATE_FUNCTION_INDEX:
			if (G_ActivateMeterSuccess == TRUE)
			{
				LcdDataBuffer.LCDBufferC[3] = DATA_BYTE_NUM_9;
				LcdDataBuffer.LCDBufferC[4] = PACKAGE_HEAD;
				LcdDataBuffer.LCDBufferC[5] = 'O'; // 'O'
				LcdDataBuffer.LCDBufferC[6] = 'K'; // 'K'
				LcdDataBuffer.LCDBufferC[7] = PACKAGE_TAIL;					
			}
			else
			{
				LcdDataBuffer.LCDBufferC[3] = DATA_BYTE_NUM_9;
				LcdDataBuffer.LCDBufferC[4] = 0;
				LcdDataBuffer.LCDBufferC[5] = 0; 
				LcdDataBuffer.LCDBufferC[6] = 0; 
				LcdDataBuffer.LCDBufferC[7] = 0;
			}							
			break;	
		case	ACTIVATECODE_STATUS_FUNCTION_INDEX:
			G_EnableActivatorAskState = TRUE;
			if (CheckActivationCode() == TRUE)
			{			    
				LcdDataBuffer.LCDBufferC[3] = DATA_BYTE_NUM_9;
				LcdDataBuffer.LCDBufferC[4] = PACKAGE_HEAD;
				LcdDataBuffer.LCDBufferC[5] = 'O'; // 'O'
				LcdDataBuffer.LCDBufferC[6] = 'K'; // 'K'
				LcdDataBuffer.LCDBufferC[7] = PACKAGE_TAIL;
			}
			else
			{
				LcdDataBuffer.LCDBufferC[3] = DATA_BYTE_NUM_9;
				LcdDataBuffer.LCDBufferC[4] = 0;
				LcdDataBuffer.LCDBufferC[5] = 0; 
				LcdDataBuffer.LCDBufferC[6] = 0; 
				LcdDataBuffer.LCDBufferC[7] = 0;
			}					
			break;
		case	2:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 14;
			ReadMMasterVariable(&LcdDataBuffer.LCDBufferF[1]);
			LcdDataBuffer.LCDBufferC[12] = ReadMeterWorkStatus();
			break;
		case	4:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 9;
			ReadMMasterVariablePVCurrent(&LcdDataBuffer.LCDBufferF[1]);
			break;
		case	9:
			break;
		case	14:
			break;
		case	10:
		case	17:
		case	21:
		case	23:
		case	26:
		case	19:
		case	20:
		case	25:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER];
			for(TempC = 0; TempC < LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]; TempC++)
			{
				LcdDataBuffer.LCDBufferC[4+TempC] = *GetPara_Char(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-LCD_CHARPARA_STARTADDRESS_IIC+TempC);
			}
			break;
		case	27:
			LcdDataBuffer.LCDBufferC[4] = VERSION0;
			LcdDataBuffer.LCDBufferC[5] = VERSION1;
			LcdDataBuffer.LCDBufferC[6] = VERSION2;
			LcdDataBuffer.LCDBufferC[7] = (uint8_t)(Get_IWR1443_Version());
			LcdDataBuffer.LCDBufferC[8] = (uint8_t)(Get_IWR1443_Version()>>8);
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 10;
			break;
		case	28:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 13;
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],DATE_LENGTH);
			break;
		case	29:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 15;
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],SENSOR_SN_LENGTH);
			break;
		case	30:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 21;
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],DESCRIPTOR_LENGTH);
			break;
		case	31:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 7;
			LcdDataBuffer.LCDBufferC[4] = 0;//RebootCount;
			LcdDataBuffer.LCDBufferC[5] = GetAllUserPara_HandleMethod();	
			break;
		case	32:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 12;
			ReadMMasterVariable(&LcdDataBuffer.LCDBufferF[1]);
			LcdDataBuffer.LCDBufferC[8] = (uint8_t)(GetBestEchoAmplitude()/AMPLITUDE_COV_SAMPLE_255);						//noise-signal ratio
			LcdDataBuffer.LCDBufferC[9] = GetMarkBestWavePostion();
			LcdDataBuffer.LCDBufferC[10] = (uint8_t)(GetBestEchoDistinctiveness());						//distinguishability
			break;
		case	33:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 13;
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-2,TAG_LENGTH);
			break;
		case	43:
			LcdDataBuffer.LCDBufferF[1] = 0;
			LcdDataBuffer.LCDBufferF[2] = 4;//HART_Variables.PWM4mABias;
			LcdDataBuffer.LCDBufferF[3] = 18;//HART_Variables.PWM18mABias;
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 17;
			break;
		case	34:
		case	38:
		case	40:
		case	46:
		case	48:
		case	49:
		case	52:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]*4;
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
			break;
		case	55:
		case	56:
		case	57:
		case	58:
		case	59:
		case	60:
		case	61:
		case	62:
		case	63:
		case	64:
		case	65:
		case	66:
		case	67:
		case	68:
		case	69:
		case	70:
		case	71:
		case	72:
		case	73:
		case	74:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 13;	//字节数
			ReadParaL_LCD(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],&LcdDataBuffer.LCDBufferF[1]);
			break;
		case	75:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 6;	//字节数
			LcdDataBuffer.LCDBufferC[4] = GetPara_PointsNum();
			break;
		case	80:		//传送曲线数据
		case	81:
		case	82:
		case	83:
		case	84:
			TransmitCurveInfo(&LcdDataBuffer.LCDBufferC[4],TRANSMITNUMBER_PERONCE*(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-80),TRANSMITNUMBER_PERONCE,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = TRANSMITNUMBER_PERONCE+5;
			UpdateLCDCount = UPDATELCDTIME-100;
			break;
		case	85:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 16;
			ReadMMasterVariable(&LcdDataBuffer.LCDBufferF[1]);
			LcdDataBuffer.LCDBufferF[2] = GetBestEchoAmplitude();
			LcdDataBuffer.LCDBufferC[12] = GetMarkBestWavePostion();
			LcdDataBuffer.LCDBufferC[13] = GetShockWaveInf();//(uint8_t)(GetBestEchoAmplitude()/AMPLITUDE_COV_SAMPLE_255);
			LcdDataBuffer.LCDBufferC[14] = GetMeterOutStatus();//(uint8_t)(GetBestEchoAmplitude()/AMPLITUDE_COV_SAMPLE_255);
			break;
		case	89:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 6;	//字节数
			LcdDataBuffer.LCDBufferC[4] = *GetPara_Char(26);
			break;
		case	100:
		case	101:
		case	102:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER];
			for(TempC = 0; TempC < LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]; TempC++)
			{
				LcdDataBuffer.LCDBufferC[4+TempC] = *GetPara_Char(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-77+TempC);
			}
			break;
		case	150:
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 9;
//			for(TempC = 0; TempC < 4; TempC++)
//			{
//				LcdDataBuffer.LCDBufferC[4+TempC] = *(GetPara_Float(9)+TempC);
//			}
			GetPara_Float(&(LcdDataBuffer.LCDBufferC[4]),9);
			break;
		case	PERFVIEW_FUNCTION_INDEX:  /* 液晶端发送性能变量观察命令 */
			if(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER] < PERF_DATA_BUFF_SIZE)
			{
				LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 53;
				for(TempC = 0; TempC < 12; TempC++)
				{
					PerfDataRead((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]+TempC),&LcdDataBuffer.LCDBufferF[TempC+1]);
				}
			}
			break;
		case	PERFAPPOINTCLEAR_FUNCTION_INDEX:  /* 液晶端发送性能变量指定清零命令 */
			if(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER] >= REALTIMEMEASUREMENT_INDEX)
			{
				PerfDataBuff_Clear_Max_Min(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
			}
			else
			{
				PerfDataWrite(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER],0);
			}
			break;
		case	PERFALLCLEAR_FUNCTION_INDEX:  /* 液晶端发送性能变量全部清零命令 */			    
			ClearAllPerf();
			break;
		case	READ_MULECHO_INFORMATION_INDEX:  /*  */			    
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]*8;
			Get_MulEcho_Information(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER],&LcdDataBuffer.LCDBufferC[4]);
			break;
		case	READ_PARAMODIFY_INFORMATION_INDEX:  /* 液晶端发送读参数修改记录命令 */			    
			ReadModifyParaInfo(&LcdDataBuffer.LCDBufferC[4],LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 25;
			break;
		case	READ_JUMPCHO_INFORMATION_INDEX:  /* 液晶端发送读跳变曲线信息命令 */			    
			if(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER] < 89)
			{
				ReadJumpCurveInfo(&LcdDataBuffer.LCDBufferC[4],LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
				LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 55;
			}
			else
			{
				ClearJumpCurveInfo();
				LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5;
			}
			break;
		case	CLEAR_JUMPCURVE_INDEX:  /* 液晶端发送读跳变曲线信息命令 */			    
//			SetJumpEchoInfoP(0);
			ClearJumpCurveInfo();
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5;
			break;
		case	READ_AUXIFO_INDEX:  /* 液晶端发送性能变量观察命令 */
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 33;
			for(TempC = 0; TempC < 2; TempC++)
			{
				PerfDataRead((3+TempC),&LcdDataBuffer.LCDBufferF[TempC+1]);
			}
			PerfDataRead(75,&LcdDataBuffer.LCDBufferF[3]);
			PerfDataRead(78,&LcdDataBuffer.LCDBufferF[4]);
			PerfDataRead(66,&LcdDataBuffer.LCDBufferF[5]);
			break;
		default:break;
	}
		if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDCHARPARA_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDCHARPARA_ENDADDRESS_IIC))
		{//IIC读前10个字符型扩展变量
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER];
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
		}
		else if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDCHARPARA2_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDCHARPARA2_ENDADDRESS_IIC))
		{
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER];
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-3,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
		}
		else if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDCHARPARA3_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDCHARPARA3_ENDADDRESS_IIC))
		{
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER];
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-3,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
		}
		else if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDFLOATPARA_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDFLOATPARA_ENDADDRESS_IIC))
		{//IIC读前20个浮点型扩展变量
			LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER] = 5+LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]*4;
			LCD_ReadEeproPara(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_PARANUMBER]);
		}		
}
//------------------------------------------------------------------------------------------------------
void	LcdWriteData(void)
{
	uint8_t TempC;
	if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= 10)
	   &&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < PERFVIEW_FUNCTION_INDEX))
	{
		RecordPerfToFRAM(MODFYPARATIMES_IIC);
	}
	switch(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS])
	{
		case	RETURN_CALCULATIONCODE_FUNCTION_INDEX:			     
			if ((LcdDataBuffer.LCDBufferC[4] == PACKAGE_HEAD)&&(LcdDataBuffer.LCDBufferC[13] == PACKAGE_TAIL))
			{
				for (TempC = 0; TempC < CALC_CODE_BYTE_NUM; TempC++)
				{
				          G_CalcSourceCode[TempC] = LcdDataBuffer.LCDBufferC[5 + TempC];
				}	
				if (ActivateMeter() == TRUE)
				{
					LcdDataBuffer.LCDBufferC[0] = ACTIVATECODE_GENERATE_FUNCTION_INDEX;
					LcdDataBuffer.LCDBufferC[2] = 0x00; // 从读主，从接收成功，主接收成功
					LcdDataBuffer.LCDBufferC[3] = DATA_BYTE_NUM_9;
					LcdDataBuffer.LCDBufferC[4] = PACKAGE_HEAD;
					LcdDataBuffer.LCDBufferC[5] = 'O'; // 'O'
					LcdDataBuffer.LCDBufferC[6] = 'K'; // 'K'
					LcdDataBuffer.LCDBufferC[7] = PACKAGE_TAIL;					
				}
				else
				{
					LcdDataBuffer.LCDBufferC[0] = ACTIVATECODE_GENERATE_FUNCTION_INDEX;
					LcdDataBuffer.LCDBufferC[2] = 0x00; // 从读主，从接收成功，主接收成功
					LcdDataBuffer.LCDBufferC[3] = DATA_BYTE_NUM_9;
					LcdDataBuffer.LCDBufferC[4] = 0;
					LcdDataBuffer.LCDBufferC[5] = 0; 
					LcdDataBuffer.LCDBufferC[6] = 0; 
					LcdDataBuffer.LCDBufferC[7] = 0;
				}					 	 
			}				 
			break;
		case	5:
			CreatFakeCurve(&LcdDataBuffer.LCDBufferF[1]);
		break;
		case	7:
			if(LcdDataBuffer.LCDBufferF[2] == 0)
			{
				GetPara_Float(&(LcdDataBuffer.LCDBufferC[8]),34-LCD_FLOATPARA_STARTADDRESS_IIC);
			}
			SetCurveStart_EndPoint(LcdDataBuffer.LCDBufferF[1],LcdDataBuffer.LCDBufferF[2]);
			UpdateLCDCount = UPDATELCDTIME-100;
		break;
		case	10:
		case	11:
		case	12:
		case	13:
		case	17:
		case	18:
		case	19:
		case	20:
		case	21:
		case	22:
		case	23:
		case	24:
		case	25:
		case	26:
		case	27:
			SavePara_CharType(MODIFYPARAMODE_IIC,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-LCD_CHARPARA_STARTADDRESS_IIC,LcdDataBuffer.LCDBufferC[4]);
			break;
		case	28:
		case	29:
		case	30:
			SavePara_StringType(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-LCD_CHARPARA_STARTADDRESS_IIC,&LcdDataBuffer.LCDBufferC[4]);
			break;
		case	31:
			SetAllUserPara_HandleMethod(MODIFYPARAMODE_IIC,LcdDataBuffer.LCDBufferC[4]);
			break;
		case	33:
			SavePara_StringType(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-LCD_CHARPARA_STARTADDRESS_IIC-2,&LcdDataBuffer.LCDBufferC[4]);
			break;
		case	34:
		case	35:
		case	36:
		case	37:
		case	38:
		case	39:
		case	40:
		case	41:
		case	42:
		case	46:
		case	47:
		case	48:
		case	49:
		case	50:
		case	51:
		case	52:
		case	53:
		case	54:
			SavePara_FloatType(MODIFYPARAMODE_IIC,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-LCD_FLOATPARA_STARTADDRESS_IIC, &LcdDataBuffer.LCDBufferF[1]);
			break;
		case	43:
			if(LcdDataBuffer.LCDBufferF[1] == 1)
			{
				ModifyCurrent(MODIFYPARAMODE_IIC);
				LcdDataBuffer.LCDBufferF[1] = 18;
			}
				
			if((LcdDataBuffer.LCDBufferF[1] < MINCURRENT_SET)||(LcdDataBuffer.LCDBufferF[1] > MAXCURRENT_SET))
				LcdDataBuffer.LCDBufferF[1] = 0;
//			else
				SetOutCurrentVal(LcdDataBuffer.LCDBufferF[1]);
			break;
		case	44:
			Set_PWM4mABias_Value(LcdDataBuffer.LCDBufferF[1]);
			break;
		case	45:
			Set_PWM18mABias_Value(LcdDataBuffer.LCDBufferF[1]);
			break;
		case	55:
		case	56:
		case	57:
		case	58:
		case	59:
		case	60:
		case	61:
		case	62:
		case	63:
		case	64:
		case	65:
		case	66:
		case	67:
		case	68:
		case	69:
		case	70:
		case	71:
		case	72:
		case	73:
		case	74:
			SavePara_LineType(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS],&LcdDataBuffer.LCDBufferF[1]);
			break;
		case	75:
			SavePara_PointsNum(LcdDataBuffer.LCDBufferC[4]);
//			HART_Variables.FlagOfChange_JulixianxingArray = PARA_CHANGED;
			break;
		case	89:
			SavePara_CharType(MODIFYPARAMODE_IIC,26,LcdDataBuffer.LCDBufferC[4]);
			break;
		case	100:
			SavePara_CharType(MODIFYPARAMODE_IIC,23,LcdDataBuffer.LCDBufferC[4]);
			break;
		case	101:
			SavePara_CharType(MODIFYPARAMODE_IIC,24,LcdDataBuffer.LCDBufferC[4]);
			break;
		case	102:
			Save_Eeprom_EnablePara_Protect(&LcdDataBuffer.LCDBufferC[4]);
			break;
		case	150:
			SavePara_FloatType(MODIFYPARAMODE_IIC,9,&LcdDataBuffer.LCDBufferF[1]);
			break;
		default:break;
	}	
		if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDCHARPARA_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDCHARPARA_ENDADDRESS_IIC))
		{
			SavePara_CharType(MODIFYPARAMODE_IIC,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-EXPANDCHARPARA_STARTADDRESS_IIC+CHAREXPANDPARA_STARTA,LcdDataBuffer.LCDBufferC[4]);
		}
		else if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDCHARPARA2_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDCHARPARA2_ENDADDRESS_IIC))
		{
			SavePara_CharType(MODIFYPARAMODE_IIC,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-EXPANDCHARPARA_STARTADDRESS_IIC+CHAREXPANDPARA_STARTA-3,LcdDataBuffer.LCDBufferC[4]);
		}
		else if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDCHARPARA3_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDCHARPARA3_ENDADDRESS_IIC))
		{
			SavePara_CharType(MODIFYPARAMODE_IIC,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-EXPANDCHARPARA_STARTADDRESS_IIC+CHAREXPANDPARA_STARTA-3,LcdDataBuffer.LCDBufferC[4]);
		}
		else if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] >= EXPANDFLOATPARA_STARTADDRESS_IIC)&&(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS] < EXPANDFLOATPARA_ENDADDRESS_IIC))
		{
			SavePara_FloatType(MODIFYPARAMODE_IIC,LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_ADDRESS]-EXPANDFLOATPARA_STARTADDRESS_IIC+FLOATEXPANDPARA_STARTA, &LcdDataBuffer.LCDBufferF[1]);
		}		
}
//------------------------------------------------------------------------------------------------------
void	WriteLcd(void)
{
	if(HAL_I2C_Master_Transmit_DMA(&hi2c1, (uint16_t)LCD_ADDRESS, (uint8_t *)LcdDataBuffer.LCDBufferC,LcdByteNumber) != HAL_OK)
	{
		/* Reading process Error */
//		_Error_Handler(__FILE__, __LINE__);
		MX_I2C1_Init();
	}
}
//-----------------------------------------------------------------------------------------------------
void	ReadLcd(void)
{
	if(HAL_I2C_Master_Receive_DMA(&hi2c1, (uint16_t)LCD_ADDRESS, (uint8_t *)LcdDataBuffer.LCDBufferC, LCDBUFFERSIZE) != HAL_OK)
	{
		/* Reading process Error */
//		_Error_Handler(__FILE__, __LINE__);
		MX_I2C1_Init();
	}
}
//------------------------------------------------------------------------------------------------------
void	AnalyzeLcdData(void)
{
	LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_CMD] &= 0xFE;
	if((LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_CMD]&0x0C) != 0x0C)
	{
		LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_CMD] &= 0xFD;
		if(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_CMD] == LCDREADDATA)
		{
			LcdReadData();
		}
		else if(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_CMD] == LCDWRITEDATA)
		{
			LcdWriteData();
		}
		CRCResult = HAL_CRC_Calculate(&hcrc,(uint32_t *)LcdDataBuffer.LCDBufferC,(LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER]-1));
		LcdDataBuffer.LCDBufferC[LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER]] = CRCResult;
		LcdDataBuffer.LCDBufferC[LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER]-1] = CRCResult>>8;
		LcdByteNumber = LcdDataBuffer.LCDBufferC[IIC_LCD_DATABUFF_BYTENUMBER]+2;
		WriteLcd();
	}
}

void	SoftwareResetI2C1(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    	__HAL_I2C_DISABLE(&hi2c1);
	hi2c1.Instance->CR1 = 0;
    	__HAL_I2C_DISABLE(&hi2c1);
	hi2c1.Instance->CR2 = 0;
    	__HAL_I2C_DISABLE(&hi2c1);
	HAL_I2C_MspDeInit(&hi2c1);
	
	GPIO_InitStruct.Pin = LCD_SCL_Pin|LCD_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, LCD_SCL_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, LCD_SDA_Pin, GPIO_PIN_SET);

	hi2c1.State = HAL_I2C_STATE_RESET;
	hi2c1.Lock = HAL_UNLOCKED;
	__HAL_I2C_ENABLE(&hi2c1);
	MX_I2C1_Init();
}
//-------------------------------------------------------------------------------------------------------------------------
void	LcdHandle(void)
{
//	uint16_t CRCResult;
	if(UpdateLCDCount >= UPDATELCDTIME)//&&(Get_IWR1443_Status() == IWR1443_WAIT_STATUS))
	{
		UpdateLCDCount = 0;
		SoftwareResetI2C1();
		if(Lcd_Read_Or_Write == READLCD)
		{
			if(LcdDataBuffer.LCDBufferC[3] >= 5)
			{
				IIcComWorkCount++;
				if(IIcComWorkCount > MAXRESETPERF_COUNT)
				{
					ResetPerfData(IICCOMERRORTIMES_TH);
					IIcComWorkCount = 0;
				}
				RecordPerfToFRAM(IICCOMTIMES_TOTAL);
				CRCResult = HAL_CRC_Calculate(&hcrc,(uint32_t *)LcdDataBuffer.LCDBufferC,(LcdDataBuffer.LCDBufferC[3]-1));
				if(CRCResult == (LcdDataBuffer.LCDBufferC[LcdDataBuffer.LCDBufferC[3]-1]*256+LcdDataBuffer.LCDBufferC[LcdDataBuffer.LCDBufferC[3]]))
				{
					AnalyzeLcdData();
				}
				else
				{
//					LcdCom_CRCErrorCount++;
					RecordPerfToFRAM(IICCOMERRORTIMES);
					RecordPerfToFRAM(IICCOMERRORTIMES_TH);
				}
			}
			else
			{
				
			}
			Lcd_Read_Or_Write = WRITELCD;
		}
		else
		{
			InitLcdData();
			ReadLcd();
			Lcd_Read_Or_Write = READLCD;
		}
	}
}
