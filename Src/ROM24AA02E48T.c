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



/****************************24AA02E48T.c文件说明*************************************************/
//该文件为加密芯片24AA02E48T相关驱动函数
/*************************************************************************************************/


#include "CopyRightActivate.h"
#include "ROM24AA02E48T.h"
#include "perf_management.h"

#define  IICROM_SDAIN_H				1

#define  IICROM_GENERATE_ACK			0
#define  IICROM_UNGENERATE_ACK			1


bool8   G_IICROMDeviceAck = FALSE;   // 设备应答，FALSE代表无应答，TRUE代表有应答


/** void  IICRomDelay(uint16  DelayTime)     //函数名称和参量和返回值类型。
* 变量范围:Time为延时时间参数，0 ≤ DelayTime ≤ 65535
*
* 返回值说明: void
*
* 功能介绍: 延时函数
*
* 关键备注：
*
* 未来提高:
* 
*/
void  IICRomDelay(uint16  DelayTime)
{ 
	uint32_t TempTimeStart;  // 循环变量
	
	TempTimeStart = HAL_GetTick();
	while((HAL_GetTick()-TempTimeStart) < DelayTime)
	{
		;
	}
	
}

GPIO_PinState	IICROM_SDAIN(void)
{
	return(HAL_GPIO_ReadPin(IICROM_SDA_GPIO_Port,IICROM_SDA_Pin));
}

void	Set_IICROM_SCL_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = IICROM_SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(IICROM_SCL_GPIO_Port, &GPIO_InitStruct);
}

void	Set_IICROM_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = IICROM_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(IICROM_SDA_GPIO_Port, &GPIO_InitStruct);
}

void	Set_IICROM_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = IICROM_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(IICROM_SDA_GPIO_Port, &GPIO_InitStruct);
}

/**void  IICROM_Inital(void)    //函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: void
*
* 功能介绍: I2C初始化，将PB6(SCK)、PB7(SDA)设置为普通输出模式，并置低电平
*
* 关键备注：
*
* 未来提高:
* 
*/
void	IICROM_Inital(void)
{
	Set_IICROM_SDA_OUT();
	Set_IICROM_SCL_OUT();
	IICROM_SCL_L;
	IICROM_SDA_L;
	IICRomDelay(IICROM_DELAY_6US);
}


/**void  Start_IICROM(void)    //函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: void
*
* 功能介绍: 产生I2C起始信号，SCL为高电平时，SDA由高电平向低电平跳变，开始传送数据
*
* 关键备注：
*
* 未来提高:
* 
*/
void	Start_IICROM(void)
{
	IICROM_Inital();
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SDA_H;
	IICRomDelay(IICROM_DELAY_6US); 
	IICROM_SCL_H;
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SDA_L; 
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SCL_L;   
	IICRomDelay(IICROM_DELAY_6US);
}


/**void  Stop_IICROM()    //函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: void
*
* 功能介绍:产生I2C停止信号，SCL为高电平时，SDA由低电平向高电平跳变，结束传送数据
*
* 关键备注：
*
* 未来提高:
* 
*/
void	Stop_IICROM()
{
	Set_IICROM_SDA_OUT();
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SDA_L;  
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SCL_H;   
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SDA_H;  
	IICRomDelay(IICROM_DELAY_6US);
}


/**bool8 Wait_Ack_IICROM()   //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8，FALSE表示设备（加密芯片）无应答，TRUE表示设备（加密芯片）有应答
*
* 功能介绍: Wait_Ack_IIROM等待应答信号到来。接收设备在第九个时钟周期会发出低电平应答信号，以表明它已收到地址或数据。
*
* 关键备注：
*
* 未来提高:
* 
*/ 
bool8	Wait_Ack_IICROM()
{
	IICROM_SDA_H;         
	Set_IICROM_SDA_IN();
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SCL_H;
	IICRomDelay(IICROM_DELAY_6US);
	if (IICROM_SDAIN() == IICROM_SDAIN_H)
	{
		IICROM_SCL_L;
		Set_IICROM_SDA_OUT();
		return FALSE;
	}
	else
	{
		IICROM_SCL_L;
		Set_IICROM_SDA_OUT();
		return TRUE; 
 	 }        
}


/**void  IIROMSendByte(uchar8 Data)    //函数名称和参量和返回值类型。
* 变量范围:Data为发送的字节数据，0 ≤ Data ≤ 255 
*
* 返回值说明: void
*
* 功能介绍: 发送一个字节，返回从机有无应答信号，Ack = 1有应答，Ack = 0无应答
*
* 关键备注：传输数据时，只有在SCL为高电平时，SDA上的电平为有效数据。
*           编写单片机向总线送数据程序时则可以在SCL还在低电平时，把数据电平送到SDA，
*           然后拉高SCL，这时SDA不应有电平跳变，延时后拉低SCL，再进行下一位的数据传送直到完成。
*
* 未来提高:
* 
*/
void	IIROMSendByte(uchar8 Data)
{
	uchar8 i;  // 循环变量
      
	Set_IICROM_SDA_OUT();
	for (i = 0; i < BYTELENGTH; i++)
	{ 
		if ( (Data & 0x80) == 0 )
		{
			IICROM_SDA_L;
		}
		else
		{
			IICROM_SDA_H;
		}
		IICRomDelay(IICROM_DELAY_6US);   
		IICROM_SCL_H;         
		Data <<= 1;
		IICRomDelay(IICROM_DELAY_6US);       
		IICROM_SCL_L;
		IICRomDelay(IICROM_DELAY_6US); 
	}     
	G_IICROMDeviceAck = Wait_Ack_IICROM();
}


/**uchar8 IIROMRcvByte()   //函数名称和参量和返回值类型。
* 变量范围: 无 
*
* 返回值说明: uchar8，范围0 ≤ ReceivedData ≤ 255 
*
* 功能介绍:从24AA02E48T接收（读取）一个字节
*
* 关键备注：
*
* 未来提高:
* 
*/ 	
uchar8	IIROMRcvByte()
{
	uchar8  ReceivedData = 0;  // 接收到的数据
	uchar8  i;                 // 循环变量
      
	IICROM_SDA_H;         
	Set_IICROM_SDA_IN();
	for (i = 0; i < BYTELENGTH; i++)
	{
		IICROM_SCL_L;     
		IICRomDelay(IICROM_DELAY_6US); 
		IICROM_SCL_H;     
		IICRomDelay(IICROM_DELAY_6US);
		ReceivedData = ReceivedData << 1;
		if (IICROM_SDAIN() == IICROM_SDAIN_H)
		{
			ReceivedData += 1; 
		}
		IICRomDelay(IICROM_DELAY_6US);
	}
	IICROM_SCL_L;
	IICRomDelay(IICROM_DELAY_6US);
	  
	return(ReceivedData);
}


/**void  Ack_Out(uchar8 Data)   //函数名称和参量和返回值类型。
* 变量范围:  Data为选择信号，0 ≤ Data ≤ 1，Data为1时不产生ACK应答，为0时产生ACK应答
*
* 返回值说明: void
*
* 功能介绍: I2C应答，TempData为1时不产生ACK应答，为0时产生ACK应答
*
* 关键备注：1、接收数据的IC在接收到8bit数据后，向发送数据的IC发出特定的低电平信号，表示已经收到数据。
*           2、设备在写周期期间不会作出应答，因此可根据应答来决定写周期的完成
*
* 未来提高:
* 
*/ 
void	Ack_Out(uchar8 Data)
{
	Set_IICROM_SDA_OUT();
	if (Data == IICROM_UNGENERATE_ACK)
	{
		IICROM_SDA_H;
	}
	else
	{
		IICROM_SDA_L;
	}
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SCL_H;
	IICRomDelay(IICROM_DELAY_6US);
	IICROM_SCL_L;
}


/**bool8  IICROM_BufferWrite(uchar8 *Buffer, uchar8 WriteAddr, uchar8 NumByteToWrite)  //函数名称和参量和返回值类型。
* 变量范围: WriteAddr为数据待存储的地址，0x00 ≤ WriteAddr ≤ 0x80，24AA02E48T内存空间为2Kbit（256Byte）,0x80~0xFF为写保护区，存放器件的唯一标识符
*           Buffer为指向字符型数据的地址，里面存放待存储数据的首字节
*           NumByteToWrite为待存储数据的字节数，0 ≤ NumByteToWrite ≤ 255
*           Page Write Time 3 ms, typical, Page Write Buffer:8-byte page (24AA02E48/24AA02E64)
*
* 返回值说明: bool8，存储成功返回TRUE，存储失败返回FALSE
*
* 功能介绍: 向24AA02E48T指定地址WriteAddr写入NumByteToWrite个字节数据
*
* 关键备注：1、如果在产生STOP条件之前传输大于8字节，那么地址计数器将会翻转，之前接收到的数据将会被覆盖掉
*           2、once the Stop condition is received ,an internal write cycle（最大5ms） will begin
*           3、每次写完后1个字节后，需要延时至少约1条IICRomDelay(IICROM_DELAY_5MS)，IICROM_DELAY_5MS=2500;语句才可以从24AA02E48T中读取此次写入的数据
*
* 未来提高:
* 
*/ 
bool8	IICROM_BufferWrite(uchar8 *Buffer, uchar8 WriteAddr, uchar8 NumByteToWrite)
{	
	uchar8  WrittedByteNum = 0;             // 写入的字节个数  
	  
	Start_IICROM();                          // Send START condition 	 
	IIROMSendByte(IICROM_DEVICE_ADDRESS);   // Send EEPROM address for write
	if (G_IICROMDeviceAck == FALSE)
	{
		return FALSE;	
	}
	IIROMSendByte(WriteAddr);               // Send the EEPROM's internal address to write to 
	if (G_IICROMDeviceAck == FALSE)
	{
		return FALSE;		   
	} 
      
	WrittedByteNum = 0;	  
	  
	do
	{		
		IIROMSendByte(*Buffer);             // Send the current byte 	
		if (G_IICROMDeviceAck == FALSE)
		{
			return FALSE;
		}	
		WrittedByteNum++;
		WriteAddr++;
		Buffer++;                   // Point to the next byte to be written
		if ( ((WriteAddr & 0x07) == 0) && (WrittedByteNum < NumByteToWrite) ) //The 2K EEPROM is capable of 8-byte Page Writes
		{
			Stop_IICROM(); 		 
			IICRomDelay(IICROM_DELAY_5MS);   // 一旦收到stop condition时，内部写周期开始，写周期最大5ms  		   	   
			Start_IICROM();                          // Send START condition 	 
			IIROMSendByte(IICROM_DEVICE_ADDRESS);   // Send EEPROM address for write
			if (G_IICROMDeviceAck == FALSE)
			{
				return FALSE;	
			}
			IIROMSendByte(WriteAddr);               // Send the EEPROM's internal address to write to 
			if (G_IICROMDeviceAck == FALSE)
			{
				return FALSE;
			}
		}		                   
	}
	while(WrittedByteNum < NumByteToWrite);         // While there is data to be written 
	Stop_IICROM();                    // Send STOP condition
	   
	return TRUE;
}


/**bool8  IICROM_BufferRead(uchar8 *Buffer, uchar8 ReadAddr, uchar8 NumByteToRead)  //函数名称和参量和返回值类型。
* 变量范围: ReadAddr为待读取数据的存储地址，0x00 ≤ ReadAddr ≤ 0xff，24AA02E48T内存空间为2Kbit（256Byte）,0x80~0xFF为写保护区，存放器件的唯一标识符
*           Buffer为指向字符型数据的地址，里面存放所需读取数据的首字节
*           NumByteToRead为待读取数据的字节数，0 ≤ NumByteToRead≤ 8，24AA02E48T的8Byte页写模式使得主机最多可写入8Byte数据到24AA02E48T
*
* 返回值说明: bool8,读取成功返回TRUE，读取失败返回FALSE
*
* 功能介绍: 从24AA02E48T指定地址ReadAddr读取NumByteToRead个字节数据
*
* 关键备注：连续读取数据时，每读取一个字节，主机回应芯片一个低电平的ACK应答，
*           当读取完最后一个字节时，主机不应答，
*
* 未来提高:
* 
*/ 
bool8	IICROM_BufferRead(uchar8 *Buffer, uchar8 ReadAddr, uchar8 NumByteToRead)
{  	
	Start_IICROM();                            // Send START condition 	
	IIROMSendByte(IICROM_DEVICE_ADDRESS);      // Send EEPROM address for write 
	if (G_IICROMDeviceAck == FALSE)
	{
		return FALSE;	
	}
	IIROMSendByte(ReadAddr);                   // Send the EEPROM's internal address to Read From 
	if (G_IICROMDeviceAck == FALSE)
	{
		return FALSE;
	}
  
	Start_IICROM();                            // Send STRAT condition a second time  	 
	IIROMSendByte(IICROM_DEVICE_ADDRESS + 1);  // Send EEPROM address for read
	if (G_IICROMDeviceAck == FALSE)
	{
		return FALSE;
	}
       
	NumByteToRead--;        
	while (NumByteToRead)                     //While there is data to be read 
	{		
		*Buffer = IIROMRcvByte();             // Read a byte from the EEPROM 		
		Buffer++;                             // Point to the next location where the byte read will be saved 
		Ack_Out(IICROM_GENERATE_ACK);         //generate ack		
		NumByteToRead--;                      // Decrement the read bytes counter 
	}
      
	*Buffer = IIROMRcvByte();                 // Read the last byte from the EEPROM
	Ack_Out(IICROM_UNGENERATE_ACK);
	Stop_IICROM();
	return TRUE;
}


/**void IICROM_WriteCharData(uchar8 SaveAddress,uchar8 Data)   //函数名称和参量和返回值类型。
* 变量范围: SaveAddress为存储地址，0 ≤ SaveAddress ≤ 0x80
*           Data为待存储的数据，0 ≤ Data ≤ 255 
*
* 返回值说明: void
*
* 功能介绍: 写一个字节字符型数据Data到24AA02E48T的SaveAddress地址
*
* 关键备注：
*
* 未来提高:
* 
*/ 
void	IICROM_WriteCharData(uchar8 SaveAddress, uchar8 Data)
{	
	IICROM_BufferWrite(&Data,SaveAddress,CHAR_BYTENUM);	
}


/**void IICROM_ReadCharData(uchar8 SaveAddress, uchar8 * Data)   //函数名称和参量和返回值类型。
* 变量范围: SaveAddress为存储地址，0 ≤ SaveAddress ≤ 0xff
*           Data为指向字符型数据的指针，里面存放读取出来的数据首字节  
*
* 返回值说明: void
*
* 功能介绍: 从24AA02E48T的SaveAddress地址中读一个字节字符型数据，放在地址Data中
*
* 关键备注：
*
* 未来提高:
* 
*/ 
void	IICROM_ReadCharData(uchar8 SaveAddress, uchar8 * Data)
{
	  IICROM_BufferRead(Data,SaveAddress,CHAR_BYTENUM);
}


/**bool8 IICROM_SaveCharData(uchar8 SaveAddress, uchar8 Data)   //函数名称和参量和返回值类型。
* 变量范围: SaveAddress为存储地址，0 ≤ SaveAddress ≤ 0x80
*           Data为待存储的数据，0 ≤ Data ≤ 255   
*
* 返回值说明:bool8，存储成功返回TRUE，存储失败返回FALSE

* 功能介绍: 保存字符型数据Data到24AA02E48T的SaveAddress地址，保存后再读出，如果读出值和保存的值一致，返回SUCCESS，否则返回FAIL
*
* 关键备注：1、如果一次存储不成功，则再存储一次，若存储3次均不成功，则返回0（FALSE）
*           2、正常执行一次该函数（一次写成功）约6ms
*
* 未来提高:
* 
*/ 
bool8	IICROM_SaveCharData(uchar8 SaveAddress, uchar8  Data)
{
	uchar8 EEPTemp;  // 临时变量，用于存放从AT24CXX中读出的数据
	uchar8 i;        // 循环变量

	for (i = 0; i < IICROM_TRY_TIME; i++)
	{	
		IICROM_WriteCharData(SaveAddress, Data);
		IICRomDelay(IICROM_DELAY_5MS);  // 每次写完数据后，由于写数据时发送完停止命令后，会进入一个内部写周期，内部写周期(字节写或页写)最大5ms
		                                    // 因此需要延时一会儿再读取，经测试该处至少延时5ms
											// The write cycle time tWR is the time from a valid stop condition 
	                                        // of a write sequence to the end of the internal clear/write cycle.		
		IICROM_ReadCharData(SaveAddress,&EEPTemp);
		if (EEPTemp == Data) 
		{
			/*	RecordEventFaultNumber(&G_EncryptPerfV,FAULT_UNHAPPEN);
				PerfDataWrite(INDEX_12, G_EncryptPerfV.BPFaultCount);	
				PerfDataWrite(INDEX_13, G_EncryptPerfV.MPFaultCount);	
				PerfDataWrite(INDEX_14, G_EncryptPerfV.SPFaultCount);*/	
			return TRUE;
		}
		else
		{
			if (SaveAddress < WRITE_PROTECT_START_ADDRESS)
			{			
				/*	WriteAccumulativePerfDataIntoPerfBuffer(INDEX_15, &G_EncryptWriteFailCount);
									
					RecordEventFaultNumber(&G_EncryptPerfV,FAULT_HAPPEN);
					PerfDataWrite(INDEX_12, G_EncryptPerfV.BPFaultCount);	
					PerfDataWrite(INDEX_13, G_EncryptPerfV.MPFaultCount);	
					PerfDataWrite(INDEX_14, G_EncryptPerfV.SPFaultCount);*/
			}			 
		}
	}		
	return FALSE;
}


/**bool8  IICROM_LoadCharData(uchar8 SaveAddress, uchar8 * Data)   //函数名称和参量和返回值类型。
* 变量范围: SaveAddress为存储地址，0 ≤ SaveAddress ≤ 0xff
*           Data为指向字符型数据的指针，里面存放读取出来的数据首字节  
*
* 返回值说明: bool8，读取成功返回TRUE，失败返回FALSE
*
* 功能介绍: 从24AA02E48T的SaveAddress地址中读一个字节数据，放在地址Data中
*
* 关键备注：如果一次读取不成功，则再读取一次，若读取3次均不成功，则返回0（FAIL）
*
* 未来提高:
* 
*/ 
bool8	IICROM_LoadCharData(uchar8 SaveAddress, uchar8* Data)
{
	uchar8 EEPTemp;  // 临时变量，用于存放从AT24CXX中读出的数据
	uchar8 i;        // 循环变量	
		
	for (i = 0;i < IICROM_TRY_TIME;i++)
	{
		IICROM_ReadCharData(SaveAddress,Data);	
		IICROM_ReadCharData(SaveAddress, &EEPTemp);
		if (EEPTemp == *Data) 
		{
			 /*  RecordEventFaultNumber(&G_EncryptPerfV,FAULT_UNHAPPEN);
			   PerfDataWrite(INDEX_12, G_EncryptPerfV.BPFaultCount);	
			   PerfDataWrite(INDEX_13, G_EncryptPerfV.MPFaultCount);	
			   PerfDataWrite(INDEX_14, G_EncryptPerfV.SPFaultCount);*/
			return TRUE;
		}
		else
		{			   
			   /*WriteAccumulativePerfDataIntoPerfBuffer(INDEX_16, &G_EncryptReadFailCount);
			   	
			   RecordEventFaultNumber(&G_EncryptPerfV,FAULT_HAPPEN);
			   PerfDataWrite(INDEX_12, G_EncryptPerfV.BPFaultCount);	
			   PerfDataWrite(INDEX_13, G_EncryptPerfV.MPFaultCount);	
			   PerfDataWrite(INDEX_14, G_EncryptPerfV.SPFaultCount);*/
		}
	}		
	return FALSE;
}


/**bool8 IICROM_SaveCharArrayData(uchar8 SaveAddress, uchar8 * Data, uint16 ArrayLength)   //函数名称和参量和返回值类型。
* 变量范围: SaveAddress为存储地址，0 ≤ SaveAddress ≤ 0x80
*           Data为待存储的数据，0 ≤ Data ≤ 255   
*           ArrayLength为待保存的字符型数组的长度，0 ≤ ArrayLength ≤ 65535
*
* 返回值说明: bool8，存储成功返回TRUE，存储失败返回FALSE
*
* 功能介绍: 保存字符型数据Data到24AA02E48T的SaveAddress地址，保存后再读出，如果读出值和保存的值一致，返回OK，否则返回FAIL
*
* 关键备注：
*
* 未来提高:
* 
*/ 
bool8	IICROM_SaveCharArrayData(uchar8 SaveAddress, uchar8 * Data, uint16 ArrayLength)
{
	uint16 i;   // 循环变量
	uchar8 ReadDataBuffer[16];
	
	IICROM_BufferWrite(Data, SaveAddress, ArrayLength);
	RecordPerfToFRAM(WRITEENCRYPTIONCHIPCOUNT);
	IICRomDelay(IICROM_DELAY_5MS);
	IICROM_BufferRead(ReadDataBuffer,SaveAddress,ArrayLength);

	for (i = 0; i < ArrayLength; i++)
	{
		if (ReadDataBuffer[i] != Data[i])
		{
			if(SaveAddress != WRITE_PROTECT_SAVE_ADDRESS)
			{
				RecordPerfToFRAM(WRITEENCRYPTIONCHIPERRORCOUNT);
			}
			return FALSE;		
		}
	}
	return TRUE;
}

/**bool8  IICROM_LoadCharArrayData(uchar8 SaveAddress, uchar8 * Data, uint16 ArrayLength)   //函数名称和参量和返回值类型。
* 变量范围: SaveAddress为存储地址，0 ≤ SaveAddress ≤ 0xff
*           Data为指向字符型数据的指针，里面存放读取出来的数据首字节  
*           ArrayLength为待读取的字符型数组的长度，0 ≤ ArrayLength ≤ 65535
*
* 返回值说明: bool8，读取成功返回TRUE，失败返回FALSE
*
* 功能介绍: 从24AA02E48T的SaveAddress地址中读ArrayLength个字节数据，放在地址Data~Data+ArrayLength-1中
*
* 关键备注：
*
* 未来提高:
* 
*/ 
bool8	IICROM_LoadCharArrayData(uchar8 SaveAddress, uchar8 * Data, uint16 ArrayLength)
{
	uint16 i;    // 循环变量
	
	RecordPerfToFRAM(READENCRYPTIONCHIPCOUNT);
	for (i = 0; i < ArrayLength; i++)
	{
		if (IICROM_LoadCharData(SaveAddress + i, (Data + i)) != TRUE)
		{			
			RecordPerfToFRAM(READENCRYPTIONCHIPERRORCOUNT);
			return FALSE;
		}		
	}
	return TRUE;
}














