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


/*
  本文件主要是性能管理相关文件。
  1、构建一个全局变量的float列表。这个列表是产品内部的一个统一的列表。这个float变量允许液晶或者通讯协议来进行读取。
     性能变量列表提供了产品性能对外的统一的一个接口。使得液晶、通讯等对性能参数的访问接口统一化。降低了访问性能变量的难度。
     内部的其它模块自己要构建自己的评测输出变量，并且自己定期更新这些变量。
     产品的整合模块部分把各个模块的评测变量的数值更新到对应的性能变量表中去。
 2、 性能管理最终的结果都体现在一系列的变量上。这些变量统一存放在一起，有统一的访问接口，就构成了性能变量表。
     性能变量表是监测结果不断把数据更新过来，而不是共用一个变量，所以性能变量表本身不需要做数据的保护。
     性能变量表需要有个excel表格来进行管理。按照序号把变量进行合理的分配给各个模块。
     性能变量表的存储和访问的接口函数对变量的意义都不关心，只是对变量的存储和访问来负责。
 3、 性能变量表就是一个float型的数组。大概空间有50个应该就足够了。
     对于每个仪表都会有的部分或者模块的内容可以放到前面。比如 数据管理器、事件管理器、和电压电流、主通讯链路等信息。其他信息可以放到后面。
*/

#include "perf_management.h"
#include "FRAM.h"
#include "Eeprom.h"
#include "EchoHandle.h"
#include "DistanceDataHandle.h"
#include "IWR1443.h"

float32  G_PerfDataManagementBuff[PERF_DATA_BUFF_SIZE + 2] = {0};   // 性能数据管理缓冲数组，即性能变量列表，允许液晶或者通讯协议来进行读取
uint16 	HSRebootTime_Index;
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

/**bool8  PerfDataBuff_Clear_Max_Min(uint16 Index)//函数名称和参量和返回值类型。
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

/**void  UpdateAccumulativePerfData(uint16 Index) //函数名称和参量和返回值类型。
* 变量范围:  Index:  0 ≤ Index < PERF_DATA_BUFF_SIZE   
*
* 返回值说明: 
*
* 功能介绍: 更新累加性质的性能变量，将累加性质的性能变量进行+1的操作
*
* 关键备注：
* 
* 未来提高: 
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
{/*记录总工作时间*/
	if((G_PerfDataManagementBuff[METERWORKTIME_L_INDEX] == 0)&&(G_PerfDataManagementBuff[METERWORKTIME_H_INDEX] == 0))
	{
	}
	UpdateAccumulativePerfData(METERWORKTIME_L_INDEX);
	if(G_PerfDataManagementBuff[METERWORKTIME_L_INDEX] > 100000)
	{
		UpdateAccumulativePerfData(METERWORKTIME_H_INDEX);
		PerfDataWrite(METERWORKTIME_L_INDEX,0);
	}
	/*记录前5个性能变量写入铁电*/
	WriteFRAM(0,20,(uint8_t *)(&G_PerfDataManagementBuff[0]));
	/*记录最近10次硬重启和软重启时间*/
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
{/*记录最后一次修改参数时间*/
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
	/*读最近10次硬重启和软重启时间*/
	ReadFRAM(400,160,(uint8_t *)(&G_PerfDataManagementBuff[100]));
	
	SetOneSecondEvenValure(G_PerfDataManagementBuff[REALTIMEMEASUREMENT_INDEX]);
}

void	RecordRebootTimeToFRAM(uint16 RecordIndex)
{/*记录最近10次硬重启和软重启时间*/
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
