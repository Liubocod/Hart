/**************************************************************************/

/**************************************************************************/

/****** CONNETECH Electronics Ltd(BeijingRuiDaYiBiaoYouXianGongSi)  ****************/

/**************************************************************************/

/**************************************************************************/

/* Declaration:        */

/*  We (CONNETECH Electronics Ltd，BeijingRuiDaYiBiaoYouXianGongSi)  has the copyright of the following software  */

/*  all the copyright, patent, trademark, trade secret or any other intellectual property infringement of following software belong to CONNETECH Electronics Ltd，BeijingRuiDaYiBiaoYouXianGongSi */

/*  all the copyright, patent, trademark, trade secret or any other intellectual property infringement of following software are the top secret of CONNETECH Electronics Ltd(BeijingRuiDaYiBiaoYouXianGongSi) */

/*  All the behaviors to read, use, copy, change, publish the software are illegal. And all rights reserved */




#ifndef PERF_MANAGEMENT_H
#define PERF_MANAGEMENT_H


#include "datatype.h"


/* 
性能管理模块调用说明:
1、其它模块的性能变量直接用数组G_PerfDataManagementBuff[]中的元素表示(使用时在每个模块的.c中用宏定义
   的方式来定义，如#define HardwareRebootCount  G_PerfDataManagementBuff[DATA_INDEX_0])，
   注意调用G_PerfDataManagementBuff[]时，数组下标不要使用指针的形式，以免溢出引起系统其它变量的
   更改，数组下标采用固定数值的形式
2、某些特殊的性能变量，如软重启次数(在软重启时，变量的值=重启前的值，单独定义一个全局变量)，这类变量就需要通过PerfDataWrite(uint16 Index, float32 Data)
   写入到性能变量表中，并且在指定性能变量清零和全部变量清零时，需通过PerfDataRead(uint16 Index, float32 *const Data)
   函数将对应G_PerfDataManagementBuff[]中的值回读给相应的变量
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
 /**bool8  PerfDataBuffInit(void) //函数名称和参量和返回值类型。
 * 变量范围: 
 *
 * 返回值说明:bool8 型变量，FALSE表示初始化失败
 *
 * 功能介绍: 初始化函数，将性能管理保护区的数据全部清零。
 *
 * 关键备注：
 * 
 * 未来提高: 
 * 
 */
bool8 PerfDataBuffInit(void);

/**bool8  PerfDataWrite(uint16 Index, float32 Data) //函数名称和参量和返回值类型。
* 变量范围:  Index:  0 ≤ Index < PERF_DATA_BUFF_SIZE
             Data: 浮点型数据，根据健康数据设定范围。
*
* 返回值说明:bool8 型变量，表示写入成功或失败 
*
* 功能介绍: 将健康数据写到某个序号上，负责将其它模块的健康衡量参数写入到存储区保存起来。
*
* 关键备注：为了支持液晶对mcu内部一些累加性质的性能变量进行清零，所以累加性质的性能变量，必须每次读一次性能变量然后加一，然后再写入。
* 
* 未来提高: 
* 
*/																		 
bool8 PerfDataWrite(uint16 Index, float32 Data);

/**void  UpdateAccumulativePerfData(uint16 Index) //函数名称和参量和返回值类型。
* 变量范围:  Index:  0 ≤ Index < PERF_DATA_BUFF_SIZE   
*
* 返回值说明: 
*
* 功能介绍:  更新累加性质的性能变量，将累加性质的性能变量进行+1的操作
*
* 关键备注：
* 
* 未来提高: 
* 
*/
void  UpdateAccumulativePerfData(uint16 Index);

/*void  PerfDataRead(uint16 Index, float32 *Data)  //函数名称和参量和返回值类型。
* 变量范围: Index:  0 ≤ Index < PERF_DATA_BUFF_SIZE
*           Data: 浮点型数据，根据健康数据设定范围。
*
* 返回值说明: 
*
* 功能介绍: 从某个序号上读取健康数据，将保存在健康参数存储区的各模块的健康参数读取出来。
*
* 关键备注：
* 
* 未来提高: 
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




