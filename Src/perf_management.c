/**************************************************************************/

/**************************************************************************/

/****** CONNETECH Electronics Ltd(BeijingRuiDaYiBiaoYouXianGongSi)  ****************/

/**************************************************************************/

/**************************************************************************/

/* Declaration:        */

/*  We (CONNETECH Electronics Ltd��BeijingRuiDaYiBiaoYouXianGongSi)  has the copyright of the following software  */

/*  all the copyright, patent, trademark, trade secret or any other intellectual property infringement of following software belong to CONNETECH Electronics Ltd��BeijingRuiDaYiBiaoYouXianGongSi */

/*  all the copyright, patent, trademark, trade secret or any other intellectual property infringement of following software are the top secret of CONNETECH Electronics Ltd(BeijingRuiDaYiBiaoYouXianGongSi) */

/*  All the behaviors to read, use, copy, change, publish the software are illegal. And all rights reserved */


/*
  ���ļ���Ҫ�����ܹ�������ļ���
  1������һ��ȫ�ֱ�����float�б�����б��ǲ�Ʒ�ڲ���һ��ͳһ���б����float��������Һ������ͨѶЭ�������ж�ȡ��
     ���ܱ����б��ṩ�˲�Ʒ���ܶ����ͳһ��һ���ӿڡ�ʹ��Һ����ͨѶ�ȶ����ܲ����ķ��ʽӿ�ͳһ���������˷������ܱ������Ѷȡ�
     �ڲ�������ģ���Լ�Ҫ�����Լ���������������������Լ����ڸ�����Щ������
     ��Ʒ������ģ�鲿�ְѸ���ģ��������������ֵ���µ���Ӧ�����ܱ�������ȥ��
 2�� ���ܹ������յĽ����������һϵ�еı����ϡ���Щ����ͳһ�����һ����ͳһ�ķ��ʽӿڣ��͹��������ܱ�����
     ���ܱ������Ǽ�������ϰ����ݸ��¹����������ǹ���һ���������������ܱ���������Ҫ�����ݵı�����
     ���ܱ�������Ҫ�и�excel��������й���������Űѱ������к���ķ��������ģ�顣
     ���ܱ�����Ĵ洢�ͷ��ʵĽӿں����Ա��������嶼�����ģ�ֻ�ǶԱ����Ĵ洢�ͷ���������
 3�� ���ܱ��������һ��float�͵����顣��ſռ���50��Ӧ�þ��㹻�ˡ�
     ����ÿ���Ǳ����еĲ��ֻ���ģ������ݿ��Էŵ�ǰ�档���� ���ݹ��������¼����������͵�ѹ��������ͨѶ��·����Ϣ��������Ϣ���Էŵ����档
*/

#include "perf_management.h"
#include "FRAM.h"
#include "Eeprom.h"
#include "EchoHandle.h"
#include "DistanceDataHandle.h"
#include "IWR1443.h"

float32  G_PerfDataManagementBuff[PERF_DATA_BUFF_SIZE + 2] = {0};   // �������ݹ��������飬�����ܱ����б�����Һ������ͨѶЭ�������ж�ȡ
uint16 	HSRebootTime_Index;
/**bool8  PerfDataBuffInit(void) //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ: 
*
* ����ֵ˵��:bool8 �ͱ�����FALSE��ʾ��ʼ��ʧ��
*
* ���ܽ���: ��ʼ�������������ܹ�������������ȫ�����㡣
*
* �ؼ���ע��
* 
* δ�����: 
* 
*/
bool8	PerfDataBuffInit_Max_Min(void)
{
	uchar8	TempC;
	uint16  i;
	bool8   PerformanceInitErrFlag = TRUE; 
   
	for (i = REALTIMEMEASUREMENT_INDEX; i < PERF_DATA_BUFFA_SIZE; i++)
	{
		TempC = i%3;
		if(TempC == 2)
		{
			if (PerfDataWrite(i,FLOAT_MAX) == FALSE)
			{             		  
				PerformanceInitErrFlag = FALSE; 
			}
		}
		else
		{
			if (PerfDataWrite(i,0.0) == FALSE)
			{             		  
				PerformanceInitErrFlag = FALSE; 
			}
		}
	}
	return PerformanceInitErrFlag;
}

/**bool8  PerfDataWrite(uint16 Index, float32 Data) //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  Index:  0 �� Index < PERF_DATA_BUFF_SIZE
             Data: ���������ݣ����ݽ��������趨��Χ��
*
* ����ֵ˵��:bool8 �ͱ�������ʾд��ɹ���ʧ�� 
*
* ���ܽ���: ����������д��ĳ������ϣ���������ģ��Ľ�����������д�뵽�洢������������
*
* �ؼ���ע��Ϊ��֧��Һ����mcu�ڲ�һЩ�ۼ����ʵ����ܱ����������㣬�����ۼ����ʵ����ܱ���������ÿ�ζ�һ�����ܱ���Ȼ���һ��Ȼ����д�롣
* 
* δ�����: 
* 
*/
bool8	PerfDataWrite(uint16 Index, float32 Data)
{
	float32  DataConfirm = 0.0;

	if (Index < PERF_DATA_BUFF_SIZE)
	{
		RecordRealPerfToFRAM(Data,Index);
//	    G_PerfDataManagementBuff[Index] = Data;    
	    PerfDataRead(Index,&DataConfirm);
	    if (DataConfirm == Data)
	    {
	        return TRUE;
	    }
	    else 
		{	   
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

/**bool8  PerfDataBuff_Clear_Max_Min(uint16 Index)//�������ƺͲ����ͷ���ֵ���͡�
* ������Χ: 
*
* ����ֵ˵��:bool8 �ͱ�����FALSE��ʾ��ʼ��ʧ��
*
* ���ܽ���: ��ʼ�������������ܹ�������������ȫ�����㡣
*
* �ؼ���ע��
* 
* δ�����: 
* 
*/
bool8	PerfDataBuff_Clear_Max_Min(uint16 Index)
{
	uchar8	TempC;
	bool8   PerformanceInitErrFlag = TRUE; 
   
	TempC = Index%3;
	if(TempC == 2)
	{
		if (PerfDataWrite(Index,FLOAT_MAX) == FALSE)
		{             		  
			PerformanceInitErrFlag = FALSE; 
		}
	}
	else
	{
		if (PerfDataWrite(Index,0.0) == FALSE)
		{             		  
			PerformanceInitErrFlag = FALSE; 
		}
	}
	return PerformanceInitErrFlag;
}

/**void  UpdateAccumulativePerfData(uint16 Index) //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  Index:  0 �� Index < PERF_DATA_BUFF_SIZE   
*
* ����ֵ˵��: 
*
* ���ܽ���: �����ۼ����ʵ����ܱ��������ۼ����ʵ����ܱ�������+1�Ĳ���
*
* �ؼ���ע��
* 
* δ�����: 
* 
*/
void	UpdateAccumulativePerfData(uint16 Index)
{
	if (Index < PERF_DATA_BUFF_SIZE)
	{
		if (G_PerfDataManagementBuff[Index] < FLOAT_MAX)
		{	
			G_PerfDataManagementBuff[Index]++;
		}	
	}
}

void	RecordMeterWorkTime(void)
{/*��¼�ܹ���ʱ��*/
	if((G_PerfDataManagementBuff[METERWORKTIME_L_INDEX] == 0)&&(G_PerfDataManagementBuff[METERWORKTIME_H_INDEX] == 0))
	{
	}
	UpdateAccumulativePerfData(METERWORKTIME_L_INDEX);
	if(G_PerfDataManagementBuff[METERWORKTIME_L_INDEX] > 100000)
	{
		UpdateAccumulativePerfData(METERWORKTIME_H_INDEX);
		PerfDataWrite(METERWORKTIME_L_INDEX,0);
	}
	/*��¼ǰ5�����ܱ���д������*/
	WriteFRAM(0,20,(uint8_t *)(&G_PerfDataManagementBuff[0]));
	/*��¼���10��Ӳ������������ʱ��*/
	if(HSRebootTime_Index < 139)
	{
		HSRebootTime_Index = HSRebootTime_Index+2;
		WriteFRAM(HSRebootTime_Index*4,8,(uint8_t *)(&G_PerfDataManagementBuff[HSRebootTime_Index]));
	}
}

void	RecordPerfToFRAM(uint16 RecordIndex)
{
	UpdateAccumulativePerfData(RecordIndex);
	WriteFRAM(RecordIndex*4,4,(uint8_t *)(&G_PerfDataManagementBuff[RecordIndex]));
}

void	RecordRealPerfToFRAM(float Temp_PerfData,uint16 RecordIndex)
{
	G_PerfDataManagementBuff[RecordIndex] = Temp_PerfData;
	WriteFRAM(RecordIndex*4,4,(uint8_t *)(&G_PerfDataManagementBuff[RecordIndex]));
}

void	RecordMaxPerfToFRAM(float Temp_PerfData,uint16 RecordIndex)
{
	if(G_PerfDataManagementBuff[RecordIndex] < Temp_PerfData)
	{
		G_PerfDataManagementBuff[RecordIndex] = Temp_PerfData;
		WriteFRAM(RecordIndex*4,4,(uint8_t *)(&G_PerfDataManagementBuff[RecordIndex]));
	}
}

void	GetMeterCurrentTime(float TempCurrentTime[2])
{
	TempCurrentTime[0] = G_PerfDataManagementBuff[METERWORKTIME_L_INDEX];
	TempCurrentTime[1] = G_PerfDataManagementBuff[METERWORKTIME_H_INDEX];
}

void	RecordLastModfyParaTime(void)
{/*��¼���һ���޸Ĳ���ʱ��*/
//	G_PerfDataManagementBuff[LASTMODFYPARATIME_L] = G_PerfDataManagementBuff[METERWORKTIME_L_INDEX];
//	G_PerfDataManagementBuff[LASTMODFYPARATIME_H] = G_PerfDataManagementBuff[METERWORKTIME_H_INDEX];
//	WriteFRAM(LASTMODFYPARATIME_L*4,8,(uint8_t *)(&G_PerfDataManagementBuff[LASTMODFYPARATIME_L]));
}

void	ReadPerfFormFRAM(void)
{
	ReadFRAM(0,80,(uint8_t *)(&G_PerfDataManagementBuff[0]));
	ReadFRAM(80,100,(uint8_t *)(&G_PerfDataManagementBuff[20]));
	ReadFRAM(REALTIMEMEASUREMENT_INDEX*4,220,(uint8_t *)(&G_PerfDataManagementBuff[REALTIMEMEASUREMENT_INDEX]));
	GetModifyParaInfoP(G_PerfDataManagementBuff[MODIFYPARINFO_P]);
//	SetJumpEchoInfoP(G_PerfDataManagementBuff[RECORD_JUMP_INDEX]);
	/*�����10��Ӳ������������ʱ��*/
	ReadFRAM(400,160,(uint8_t *)(&G_PerfDataManagementBuff[100]));
	
	SetOneSecondEvenValure(G_PerfDataManagementBuff[REALTIMEMEASUREMENT_INDEX]);
}

void	RecordRebootTimeToFRAM(uint16 RecordIndex)
{/*��¼���10��Ӳ������������ʱ��*/
	uint8_t	TempC;
	uint16_t TempIndex;
	ReadFRAM(0,80,(uint8_t *)(&G_PerfDataManagementBuff[0]));
	TempIndex = 20*(RecordIndex-1)+100;
	UpdateAccumulativePerfData(RecordIndex);
	for(TempC = 9; TempC > 0; TempC--)
	{
		G_PerfDataManagementBuff[TempIndex+2*TempC] = G_PerfDataManagementBuff[TempIndex+2*TempC-2];
		G_PerfDataManagementBuff[TempIndex+2*TempC+1] = G_PerfDataManagementBuff[TempIndex+2*TempC-1];		
	}
	G_PerfDataManagementBuff[TempIndex] = G_PerfDataManagementBuff[METERWORKTIME_L_INDEX];
	G_PerfDataManagementBuff[TempIndex+1] = G_PerfDataManagementBuff[METERWORKTIME_H_INDEX];
	HSRebootTime_Index = 98;
//	WriteFRAM(TempIndex*4,8,(uint8_t *)(&G_PerfDataManagementBuff[TempIndex]));
}

/*void  PerfDataRead(uint16 Index, float32 *Data)  //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ: Index:  0 �� Index < PERF_DATA_BUFF_SIZE
*           Data: ���������ݣ����ݽ��������趨��Χ��
*
* ����ֵ˵��: 
*
* ���ܽ���: ��ĳ������϶�ȡ�������ݣ��������ڽ��������洢���ĸ�ģ��Ľ���������ȡ������
*
* �ؼ���ע��
* 
* δ�����: 
* 
*/
void	PerfDataRead(uint16 Index, float32 *Data)
{
	if (Index < PERF_DATA_BUFF_SIZE)
	{
		*Data = G_PerfDataManagementBuff[Index];
	}
}

void	UpdatePerfdata_Max_Min(uint16 Index, float32 Data)
{
	PerfDataWrite(Index, Data);
	if(Data > G_PerfDataManagementBuff[Index+1])
	{
		PerfDataWrite(Index+1, Data);
	}
	if(Data < G_PerfDataManagementBuff[Index+2])
	{
		PerfDataWrite(Index+2, Data);
	}
}

bool8	ClearAllPerf(void)
{
	uchar8	TempC;
	bool8   PerformanceInitErrFlag = TRUE; 
	for (TempC = 0; TempC < REALTIMEMEASUREMENT_INDEX; TempC++)
	{
		if (PerfDataWrite(TempC,0.0) == FALSE)
		{             		  
			PerformanceInitErrFlag = FALSE; 
		}
	}
	PerfDataBuffInit_Max_Min();
	for (TempC = PERF_DATA_BUFFA_SIZE; TempC < PERF_DATA_BUFF_SIZE; TempC++)
	{
		if (PerfDataWrite(TempC,0.0) == FALSE)
		{             		  
			PerformanceInitErrFlag = FALSE; 
		}
	}
	WriteFRAM(20,180,(uint8_t *)(&G_PerfDataManagementBuff[0]));
	RecordPWMCurrentPara();
	ClearIWR1443UnWorkCount();
	ClearHighNoiseContiuing_Count();
	HSRebootTime_Index = 98;
	return PerformanceInitErrFlag;
}

void	Record_Max_MinToFRAM(void)
{
	WriteFRAM(REALTIMEMEASUREMENT_INDEX*4,180,(uint8_t *)(&G_PerfDataManagementBuff[REALTIMEMEASUREMENT_INDEX]));		
}

void	ResetPerfData(uint16 Index)
{
	G_PerfDataManagementBuff[Index] = 0;
	WriteFRAM(Index*4,4,(uint8_t *)(&G_PerfDataManagementBuff[Index]));
}
