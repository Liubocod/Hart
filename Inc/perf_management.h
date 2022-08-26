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




#ifndef PERF_MANAGEMENT_H
#define PERF_MANAGEMENT_H


#include "datatype.h"


/* 
���ܹ���ģ�����˵��:
1������ģ������ܱ���ֱ��������G_PerfDataManagementBuff[]�е�Ԫ�ر�ʾ(ʹ��ʱ��ÿ��ģ���.c���ú궨��
   �ķ�ʽ�����壬��#define HardwareRebootCount  G_PerfDataManagementBuff[DATA_INDEX_0])��
   ע�����G_PerfDataManagementBuff[]ʱ�������±겻Ҫʹ��ָ�����ʽ�������������ϵͳ����������
   ���ģ������±���ù̶���ֵ����ʽ
2��ĳЩ��������ܱ�����������������(��������ʱ��������ֵ=����ǰ��ֵ����������һ��ȫ�ֱ���)�������������Ҫͨ��PerfDataWrite(uint16 Index, float32 Data)
   д�뵽���ܱ������У�������ָ�����ܱ��������ȫ����������ʱ����ͨ��PerfDataRead(uint16 Index, float32 *const Data)
   ��������ӦG_PerfDataManagementBuff[]�е�ֵ�ض�����Ӧ�ı���
*/

#define PERF_DATA_BUFFA_SIZE			99
#define PERF_DATA_BUFF_SIZE			200

#define HARDFAULT_INDEX				0
#define HARDREBOOTCOUNT_INDEX			1
#define SOFTREBOOTCOUNT_INDEX			2
#define METERWORKTIME_L_INDEX			3
#define METERWORKTIME_H_INDEX			4

#define MODFYPARATIMES_485			5
#define MODFYPARATIMES_HART			6
#define MODFYPARATIMES_IIC			7
#define MODFYPARATIMES_BLTOOTH			8
#define MODFYPARATIMES_TOTAL			9
#define SETDEFAULTPARATIMES			10//
#define MODIFYPARINFO_P				11
#define RECORD_JUMP_INDEX			12

#define READEEPROMTOTALTIMES			13
#define READEEPROMERRORTIMES			14
#define WRITEEEPROMTOTALTIMES			15
#define WRITEEEPROMERRORTIMES			16

#define CONTROLIWR1443TIMES			17
#define NOTWORKTIMES_IWR1443			18
#define NOTWORKTIMES_IWR1443_TH			19
#define CRCERRORTIMES_IWR1443			20

#define HARTCOMTIMES_TOTAL			21
#define HARTCOMERRORTIMES			22
#define HARTCOMERRORTIMES_TH			23
#define COM485TIMES_TOTAL			24
#define COM485ERRORTIMES			25
#define COM485ERRORTIMES_TH			26
#define IICCOMTIMES_TOTAL			27
#define IICCOMERRORTIMES			28
#define IICCOMERRORTIMES_TH			29
#define BLTOOTHTIMES_TOTAL			30
#define BLTOOTHERRORTIMES			31
#define BLTOOTHERRORTIMES_TH			32

#define RAMERRORTIMES				33
#define VCON_LOWER				34
#define	PWM18MA_P				35
#define	PWM4MA_P				36
#define	CHECKACTIVETOTALCOUNT			37
#define	CHECKACTIVEERRORCOUNT			38
#define	READENCRYPTIONCHIPCOUNT			39
#define	READENCRYPTIONCHIPERRORCOUNT		40
#define	WRITEENCRYPTIONCHIPCOUNT		41
#define	WRITEENCRYPTIONCHIPERRORCOUNT		42

#define	HIGHNOISETOTALCOUNT			43
#define	HIGHNOISECONTINUNGCOUNT			44

#define REALTIMEMEASUREMENT_INDEX		45
#define VOLAGECURRENT_STARTINDEX		48
#define EXTERNALCURRENT_INDEX			75
#define EXTERNALVOLTAGE_INDEX			78
#define VCONVOLAGE_INDEX			33+VOLAGECURRENT_STARTINDEX
#define VCONVOLAGE_DV_INDEX			VCONVOLAGE_INDEX+3
#define IWR1443WORKTIME_INDEX			VCONVOLAGE_DV_INDEX+3

#define	IWR1443STATUS_1				90
#define	IWR1443STATUS_2				91
#define	IWR1443STATUS_3				92

#define MAXRESETPERF_COUNT			1000
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
bool8 PerfDataBuffInit(void);

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
bool8 PerfDataWrite(uint16 Index, float32 Data);

/**void  UpdateAccumulativePerfData(uint16 Index) //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  Index:  0 �� Index < PERF_DATA_BUFF_SIZE   
*
* ����ֵ˵��: 
*
* ���ܽ���:  �����ۼ����ʵ����ܱ��������ۼ����ʵ����ܱ�������+1�Ĳ���
*
* �ؼ���ע��
* 
* δ�����: 
* 
*/
void  UpdateAccumulativePerfData(uint16 Index);

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
void	PerfDataRead(uint16 Index, float32 *const Data);

bool8  PerfDataBuffInit_Max_Min(void);
bool8  PerfDataBuff_Clear_Max_Min(uint16 Index);
bool8	ClearAllPerf(void);

void	UpdatePerfdata_Max_Min(uint16 Index, float32 Data);
void	RecordPerfToFRAM(uint16 RecordIndex);
void	RecordMeterWorkTime(void);
void	ReadPerfFormFRAM(void);
void	RecordRebootTimeToFRAM(uint16 RecordIndex);
void	RecordLastModfyParaTime(void);
void	Record_Max_MinToFRAM(void);
void	RecordRealPerfToFRAM(float Temp_PerfData,uint16 RecordIndex);
void	ResetPerfData(uint16 Index);
void	RecordMaxPerfToFRAM(float Temp_PerfData,uint16 RecordIndex);
void	GetMeterCurrentTime(float TempCurrentTime[2]);









#endif




