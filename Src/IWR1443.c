/* Includes ------------------------------------------------------------------*/
#include "crc.h"
#include "usart.h"
#include "gpio.h"
#include "dma.h"
#include "Eeprom.h"
#include "IWR1443.h"
#include "EchoHandle.h"
#include "HandleADSample.h"
#include "perf_management.h"

extern IWR1443_Params_Trans__TypeDef IWR1443_Params_Trans;

ReceiveDataBuffer__TypeDef ReceiveDataBuffer_80G;
uint8_t IWR1443_Status,IWR1443_WorkTime,IWR1443_RData_Status,IWR1443_WorkMode;
uint32_t StartIWR1443Time,IWR1443WorkPeriod;
uint16_t IWR1443WorkTimeMax;
uint16_t CRCResult_IWR1443;
uint16_t ControlIWR1443WorkCount;
uint16_t IWR1443UnWorkCount;
uint16_t IWR1443StatusInfo[RECEIVEBUFFERSIZE_STATUS];
uint32_t TestWorkPeriod,TestTime;

uint8_t	GetIWR1443_WorkMode(void)
{
	return(IWR1443_WorkMode);
}

void	StartUart2_ReceiveIWR1443Status(void)
{
	HAL_UART_DeInit(&huart3);
	MX_USART2_UART_Init();
	if(HAL_UART_Receive_DMA(&huart2, (uint8_t *)IWR1443StatusInfo, RECEIVEBUFFERSIZE_STATUS*4) != HAL_OK)
	{
		HAL_UART_DeInit(&huart2);
		MX_USART2_UART_Init();
	}
}

void	StartUart2_ReceiveIWR1443Data(void)
{
	uint16_t TempSize;
	HAL_UART_DeInit(&huart3);
	MX_USART2_UART_Init();
	if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 0)
	{
		TempSize = RECEIVEBUFFERSIZE_80G+1;
	}
	else if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 1)
	{
		if(IWR1443_Params_Trans.IWR1443_Params.code_range < 6)
			TempSize = RECEIVEBUFFERSIZE_80G_512FFT;
		else
			TempSize = RECEIVEBUFFERSIZE_80G_1024FFT;
	}
	else if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 2)
	{
		TempSize = RECEIVEBUFFERSIZE_80G_1024FFT;
	}
	TempSize = TempSize*4;
	if(HAL_UART_Receive_DMA(&huart2, (uint8_t *)ReceiveDataBuffer_80G.RxDataBufferC, TempSize) != HAL_OK)
	{
		HAL_UART_DeInit(&huart2);
		MX_USART2_UART_Init();
	}
}

void	StartUartTransmit(void)
{
	//���͸ĳɴ���2
	IWR1443_WorkMode = IWR1443_Params_Trans.IWR1443_Params.Para_BackUp;
	if(HAL_UART_Transmit_DMA(&huart2, (uint8_t *)IWR1443_Params_Trans.IWR1443_ParamsC, sizeof(IWR1443_Params_Trans)) != HAL_OK)
	{
		HAL_UART_DeInit(&huart2);
		MX_USART2_UART_Init();
	}

/*	MX_USART3_UART_Init();
	if(HAL_UART_Transmit_DMA(&huart3, (uint8_t *)IWR1443_Params_Trans.IWR1443_ParamsC, sizeof(IWR1443_Params_Trans)) != HAL_OK)
	{
		HAL_UART_DeInit(&huart3);
		MX_USART3_UART_Init();
	}*/
}

void	StartTransmitToIWR1443(void)
{
	if(IWR1443_Status == IWR1443_START_STATUS)
	{
		IWR1443_Status = IWR1443_CONFIGING_STATUS;
		StartUart2_ReceiveIWR1443Data();//�ĳ�ͬһ�����ڽ��շ��ͣ�ͬʱ������������
		StartUartTransmit();
	}
}

void	InitIWR1443Port(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

//	GPIO_InitStruct.Pin = IRW1443_RST_Pin;
//	HAL_GPIO_Init(IRW1443_RST_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_12_Pin;
	HAL_GPIO_Init(CON_RF_POWER_12_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_13_Pin;
	HAL_GPIO_Init(CON_RF_POWER_13_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_18_Pin;
	HAL_GPIO_Init(CON_RF_POWER_18_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_22_Pin;
	HAL_GPIO_Init(CON_RF_POWER_22_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_33_Pin;
	HAL_GPIO_Init(CON_RF_POWER_33_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : BT_VCC_Pin */
	GPIO_InitStruct.Pin = BT_VCC_Pin;
	HAL_GPIO_Init(BT_VCC_GPIO_Port, &GPIO_InitStruct);
}

void	StopRFPort(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	GPIO_InitStruct.Pin = CON_RF_POWER_13_Pin;
	HAL_GPIO_Init(CON_RF_POWER_13_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_18_Pin;
	HAL_GPIO_Init(CON_RF_POWER_18_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_22_Pin;
	HAL_GPIO_Init(CON_RF_POWER_22_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CON_RF_POWER_33_Pin;
	HAL_GPIO_Init(CON_RF_POWER_33_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : BT_VCC_Pin */
	GPIO_InitStruct.Pin = BT_VCC_Pin;
	HAL_GPIO_Init(BT_VCC_GPIO_Port, &GPIO_InitStruct);
}

void	DisableIWR1443Vcc(void)
{
//	Enable_RF_NRST;
	DISABLE_RF_VCC33;
	DISABLE_RF_VCC22;
	DISABLE_RF_VCC18;
	DISABLE_RF_VCC12;
	DISABLE_RF_VCC13;
	HAL_UART_DeInit(&huart2);
//	StopRFPort();
}

void	AbnormalCloseWR1443Vcc(void)
{
	DisableIWR1443Vcc();
	IWR1443_RData_Status = IWR1443_RDATA_USED;
	IWR1443_Status = IWR1443_WAIT_STATUS;
}

void	EnableIWR1443Vcc(void)
{
//	InitIWR1443Port();
	ENABLE_RF_VCC33;
//	Disable_RF_NRST;
	ENABLE_RF_VCC13;
	ENABLE_RF_VCC12;
	ENABLE_RF_VCC18;
	ENABLE_RF_VCC22;
	MultiChannel_StartSample();
}
/*���������ݻ��߹���ʱ�䵽�����ʱ�޾�ֹͣIWR1443����*/
void	EndIWR1443Work(void)
{
	if(IWR1443_Status == IWR1443_WORKING_STATUS)
	{
		DisableIWR1443Vcc();
//		if(IWR1443_WorkTime > TRANSMITTOIWR1443)
//		{
		UpdatePerfdata_Max_Min(IWR1443WORKTIME_INDEX,IWR1443_WorkTime);
//		}
		IWR1443_Status = IWR1443_WAIT_STATUS;		//1443����ȴ�״̬
		IWR1443_WorkTime = 0;
		IWR1443_RData_Status = IWR1443_RDATA_UNUSE;
	}
	else //if(IWR1443_Status == IWR1443_WORKING_STATUS)
	{
		HAL_UART_DeInit(&huart2);
//		HAL_UART_DeInit(&huart3);
	}
}

void	InitIWR1443Para(void)
{
	InitIWR1443Port();
	AbnormalCloseWR1443Vcc();
//	IWR1443WorkTimeMax = MAXIWR1443WORKTIME_FM;
//	EndIWR1443Work();
	IWR1443_RData_Status = IWR1443_RDATA_USED;
//	UpdateConfigIWR1443Para();
	IWR1443UnWorkCount = 0;
	ControlIWR1443WorkCount = 0;
//	ReceiveStartP = 6;
//	EnableIWR1443Vcc();/*ˢ1443�����ã�����������ε�*/
//	StartIWR1443Time = HAL_GetTick();
}

float	GetIWR1443WorkPeriod(void)
{
	float TempF;
	TempF = (float)(IWR1443WorkPeriod);
	return(TempF/1000.0);
}

void	IncIWR443_WorkTime(void)
{
	if((IWR1443_Status <= IWR1443_WORKING_STATUS)&&(IWR1443_Status >= IWR1443_START_STATUS))
	{
		IWR1443_WorkTime++;//1ms
	}
}

void	ResetReceiveDataBuffer_80G(void)
{
	uint8_t TempC;
	for(TempC = 0; TempC < 2; TempC++)
	{
		ReceiveDataBuffer_80G.RxDataBufferI[TempC] = 0;
	}
	for(TempC = 28; TempC < 62; TempC++)
	{
		ReceiveDataBuffer_80G.RxDataBufferI[TempC] = 0;
	}
//	for(TempC = 0; TempC < RECEIVEBUFFERSIZE_STATUS; TempC++)
//	{
//		IWR1443StatusInfo[TempC] = 0;
//	}
}

void	HandleIWR1443_ControlFlow(void)
{
//	uint32_t TempLong;
	if(IWR1443_Status == IWR1443_WAIT_STATUS)
	{
		if(IWR1443_RData_Status == IWR1443_RDATA_UNUSE)
		{
			TestTime = HAL_GetTick();
			if((ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_LEADCODEADDRESS] == 0xA55A)&&(ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_LEADCODEADDRESS+1] == 0xA55A))
			{
				if(ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFERSIZE_80G] == HAL_CRC_Calculate(&hcrc,(uint32_t *)(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_LEADCODEADDRESS+2]),RECEIVEBUFFERSIZE_CRC))//24+6
				{
					if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 0)
					{
						EchoData_1443(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_CURVEADDRESS]);//81
					}
					else if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 1)
					{
						EchoDataHandle((int16_t *)(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFERSIZE_80G+1]));//81
					}
					else if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 2)
					{
						Sample_Data_Process((int16_t *)(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFERSIZE_80G+1]));
					}
					
					ChipInfHandle(&ReceiveDataBuffer_80G.RxDataBufferC[RECEIVEBUFFER_CHIPADDRESS]);//6
					HandleShockWave(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_CURVEADDRESS]);
					Handle_NoiseCal_EchoSW();
//					ReceiveStartP = 6;
//					IWR1443WorkTimeMax = MAXIWR1443WORKTIME_PM;
				}
				else
				{/*ͨ��У�����*/
					RecordPerfToFRAM(CRCERRORTIMES_IWR1443);
				}
				ResetReceiveDataBuffer_80G();
			}
			TestWorkPeriod = HAL_GetTick()-TestTime;
			IWR1443_RData_Status = IWR1443_RDATA_USED;
//			else
//			{/*ͨ��У�����*/
//				RecordPerfToFRAM(CRCERRORTIMES_IWR1443);
//			}
		}
//		else if(IWR1443_RData_Status == IWR1443_RDATA_TIMROUT)
//		{/*ͨ�ų�ʱ��CPUû�н��յ�IWR1443���͵�����*/
//			IWR1443_RData_Status = IWR1443_RDATA_USED;
//		}
	}
//	if(((HAL_GetTick()-StartIWR1443Time) > MAXIWR1443WORKPERIOD)||(IWR1443_Status == IWR1443_START_POWERON))
	if((IWR1443_Status == IWR1443_START_POWERON)&&(IWR1443_RData_Status == IWR1443_RDATA_USED))
	{
		RecordPerfToFRAM(CONTROLIWR1443TIMES);
		ControlIWR1443WorkCount++;
		if(ControlIWR1443WorkCount > MAXRESETPERF_COUNT)
		{
			IWR1443UnWorkCount = 0;
			ControlIWR1443WorkCount = 0;
		}
		IWR1443WorkPeriod = HAL_GetTick()-StartIWR1443Time;
		StartIWR1443Time = HAL_GetTick();
		EnableIWR1443Vcc();				//����1443���е�Դ��3.3��1.8��1.3��1.2��
		IWR1443_Status = IWR1443_START_STATUS;		//��������״̬
		IWR1443_WorkTime = 0;				//1443����ʱ������
	}
	else
	{
		if((IWR1443_WorkTime > GetRFWorkTimeMax())&&(IWR1443_Status != IWR1443_WAIT_STATUS))
		{//����IWR1443�����ʱ��
			DisableIWR1443Vcc();				//�ر�����1443�ĵ�Դ��3.3��1.8��1.3��1.2��
			IWR1443_Status = IWR1443_WAIT_STATUS;		//1443����ȴ�״̬
			IWR1443_RData_Status = IWR1443_RDATA_USED;
			if((ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_LEADCODEADDRESS] == 0xA55A)&&(ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_LEADCODEADDRESS+1] == 0xA55A))
			{
				if(ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFERSIZE_80G] == HAL_CRC_Calculate(&hcrc,(uint32_t *)(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_LEADCODEADDRESS+2]),RECEIVEBUFFERSIZE_CRC))//24+6
				{
					if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 0)
					{
						EchoData_1443(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_CURVEADDRESS]);//81
					}
					else if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 1)
					{
						EchoDataHandle((int16_t *)(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFERSIZE_80G+1]));//81
					}
					else if(IWR1443_Params_Trans.IWR1443_Params.baud_rate == 2)
					{
						Sample_Data_Process((int16_t *)(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFERSIZE_80G+1]));
					}
					ChipInfHandle(&ReceiveDataBuffer_80G.RxDataBufferC[RECEIVEBUFFER_CHIPADDRESS]);//6
					HandleShockWave(&ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_CURVEADDRESS]);
					Handle_NoiseCal_EchoSW();
//					ReceiveStartP = 6;
//					IWR1443WorkTimeMax = MAXIWR1443WORKTIME_PM;
				}
				else
				{/*ͨ��У�����*/
					RecordPerfToFRAM(CRCERRORTIMES_IWR1443);
				}
				ResetReceiveDataBuffer_80G();
			}
			else
			{
				RecordPerfToFRAM(NOTWORKTIMES_IWR1443);
				IWR1443UnWorkCount++;
//				RecordPerfToFRAM(NOTWORKTIMES_IWR1443_TH);
				RecordMaxPerfToFRAM(IWR1443UnWorkCount,NOTWORKTIMES_IWR1443_TH);
				if((IWR1443StatusInfo[0] == 0x6BB6)&&(IWR1443StatusInfo[1] == 0x6BB6))
				{
//					if(IWR1443StatusInfo[5] == HAL_CRC_Calculate(&hcrc,(uint32_t *)(&IWR1443StatusInfo[2]),6))
//					{
/*						TempLong = IWR1443StatusInfo[10]+IWR1443StatusInfo[9]*65536;
						RecordRealPerfToFRAM((float)TempLong,IWR1443STATUS_1);
						TempLong = IWR1443StatusInfo[16]+IWR1443StatusInfo[15]*65536;
						RecordRealPerfToFRAM((float)TempLong,IWR1443STATUS_2);
						TempLong = IWR1443StatusInfo[22]+IWR1443StatusInfo[21]*65536;
						RecordRealPerfToFRAM((float)TempLong,IWR1443STATUS_3);
						TempLong = IWR1443StatusInfo[28]+IWR1443StatusInfo[27]*65536;
						RecordRealPerfToFRAM((float)TempLong,93);
						TempLong = IWR1443StatusInfo[34]+IWR1443StatusInfo[33]*65536;
						RecordRealPerfToFRAM((float)TempLong,94);
						TempLong = IWR1443StatusInfo[40]+IWR1443StatusInfo[39]*65536;
						RecordRealPerfToFRAM((float)TempLong,95);*/
//					}
				}
/*				if((ReceiveDataBuffer_80G.RxDataBufferI[0] == 0x6BB6)&&(ReceiveDataBuffer_80G.RxDataBufferI[1] == 0x6BB6))
				{
					if(ReceiveDataBuffer_80G.RxDataBufferI[5] == HAL_CRC_Calculate(&hcrc,(uint32_t *)(&ReceiveDataBuffer_80G.RxDataBufferI[2]),6))
					{
						TempLong = ReceiveDataBuffer_80G.RxDataBufferI[3]+ReceiveDataBuffer_80G.RxDataBufferI[4]*65536;
						RecordRealPerfToFRAM((float)TempLong,IWR1443STATUS_3);
					}
				}*/
				ResetReceiveDataBuffer_80G();
				IWR1443_RData_Status = IWR1443_RDATA_USED;
			}
//			IWR1443_RData_Status = IWR1443_RDATA_TIMROUT;
			//
		}
		else if((IWR1443_WorkTime >= TRANSMITTOIWR1443)&&(IWR1443_Status == IWR1443_START_STATUS))
		{
//			StartUart2_ReceiveIWR1443Status();		//����Uart����IWR1443״̬����
			StartTransmitToIWR1443();
		}
		else if((IWR1443_WorkTime >= STARTIWR1443UART)&&(IWR1443_Status == IWR1443_CONFIGEND_STATUS))
		{
//			StartUart2_ReceiveIWR1443Data();		//����Uart����IWR1443����
//			HAL_SPI_Receive_DMA();				//����SPI����IWR1443����
			IWR1443_Status = IWR1443_WORKING_STATUS;	//�����������״̬
		}
	}
}

void	Set_IWR1443_Status(uint8_t CurrentStatus)
{
	IWR1443_Status = CurrentStatus;
}

uint8_t	Get_IWR1443_Status(void)
{
	return(IWR1443_Status);
}

uint16_t Get_IWR1443_Version(void)
{
	return(ReceiveDataBuffer_80G.RxDataBufferI[RECEIVEBUFFER_VERSION]);
}

void	ClearIWR1443UnWorkCount(void)
{
	IWR1443UnWorkCount = 0;
}