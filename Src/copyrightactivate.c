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



/****************************CopyRightActivate.c文件说明**********************************/
//该文件为激活产品相关函数
/*************************************************************************************************/


#include "CopyRightActivate.h"
#include "ROM24AA02E48T.h"
#include "DataType.h"
#include "crc.h"
#include "perf_management.h"
#include "Eeprom.h"


#define  COMPANY_NAME       "CONNETECH Electronics Ltd(BeijingRuiDaYiBiaoYouXianGongSi)"    // sizeof(COMPANY_NAME)=59Byte
#define  COPYRIGHT_DECLARE  "All datas in this eeprom,flash or mcu is the important property of CONNETECH(BeijingRuiDaYiBiaoYouXianGongSi).The copyrights belong to connetech (BeijingRuiDaYiBiaoYouXianGongSi),and All rights reserved.To read out or copy these datas in eeprom,flash or mcu without permission of CONNETECH is Copyright infringement."   // sizeof(COPYRIGHT_DECLARE)=317Byte


#define  FIXED_CODE_BYTE_NUM       	        CALC_CODE_BYTE_NUM + 2     /* 固定码字节数 */

#define  UNIQUE_CODE_SAVE_ADDRESS           0xFA                       /* 唯一码存储地址FAh~FFh */
#define  READ_CORRECT_UNIQUECODE_FLAG       0xA8                       /* 读取到正确的唯一码标志符 */ 

#define  FIXED_CODE1                        0xA5   /* "计算码+固定码" 中的固定码 */
#define  FIXED_CODE2                        0x5A   /* "计算码+固定码" 中的固定码 */

#define  FIXED_VALUE                        0x01   /* 由计算码计算出唯一码时 算法中用到的定值 */


uchar8    G_CompanyName_RAM[] = {COMPANY_NAME};              /* 存放在RAM中的公司名称 */
uchar8    G_CopyRightDeclare_RAM[] = {COPYRIGHT_DECLARE};    /* 存放在RAM中的版权声明 */

const uint8_t   G_CompanyName[] = {COMPANY_NAME};              /* 存放在常量存储区中的公司名称 */
const uint8_t   G_CopyRightDeclare[] = {COPYRIGHT_DECLARE};    /* 存放在常量存储区中的版权声明 */
  
uchar8    G_UniqueCode[UNIQUE_CODE_BYTE_NUM] = {0};         /* 加密芯片唯一码存放数组 */
uchar8    G_CalcSourceCode[CALC_CODE_BYTE_NUM] = {0};       /* 加密芯片计算码原码存放数组 */
uchar8    G_CalcReverseCode[CALC_CODE_BYTE_NUM] = {0};      /* 加密芯片计算码反码存放数组 */   
uchar8    G_CalcFixedCode[FIXED_CODE_BYTE_NUM] = {0};       /* 加密芯片计算码固定码存放数组 */ 

uint16_t    G_CompanyNameReadByteNum1 = 0;                    /* 从EEPROM中读取的公司名称1字节数 */
uint16_t    G_CompanyNameReadByteNum2 = 0;                    /* 从EEPROM中读取的公司名称2字节数 */
uint16    G_CopyRightReadByteNum = 0;                       /* 从EEPROM中读取的版权声明字节数 */


uint16  G_CheckActivateCodeTimer = 0;     /* 定期检查激活码(计算码、公司名称、版权声明)用的计时变量,调用该文件时将该变量放在定时器中断中进行++
                                             当 >= CHECK_ACTIVATIONCODE_TIME 时进行一次激活码检查 */
uchar8  G_ActivateCodeErrorCount = 0;     /* 激活码(计算码、公司名称、版权声明)错误次数,检查到激活码错误时该变量+1
                                             当 >= CHECK_ACTIVATIONCODE_FAIL_NUM时启用报复行为 */

bool8   G_EnableActivatorAskState = FALSE;  /* 询问激活状态使能，检查激活码时分两种，一种是系统定时检查，把整体检查的工作量平均分配到每一次程序调用中
                                               另外一种是当仪表的激活状态被询问时，把所有激活信息检查一下 */

bool8   G_EnableRevenge = FALSE; /* 报复使能，当激活码检查失败一定次数后，该变量为TRUE，TRUE表示使能报复行为（仪表工作在非正常状态，具体状态由调用者决定），
                                    FALSE表示不使能报复行为 */

bool8   G_ActivateMeterSuccess = FALSE; /* 激活仪表成功标志位 */

bool8   G_AllActivateCodeCheckFinish = FALSE; /* 所有激活码检查完毕，平时检查激活码时EEPROM中的数据是分时分段检查的，
                                                 每隔一定的时间检查一遍激活码(每次进入主循环时检查一段激活码)，
                                                 待所有激活码检查一遍后，再复位检查计时变量 */
bool8   G_AllEEpromDataCheckSuccess = TRUE;  /* 所有EEPROM数据检查成功，平时检查激活码时EEPROM中的数据是分时分段检查的，
                                                每隔一定的时间检查一遍激活码(每次进入主循环时检查一段激活码)，
                                                待所有激活码检查一遍后，再复位检查计时变量 */
uint32_t TestTime_Start,TestTime_End;




/** bool8    ReadUniqueCode(uchar8  UniqueCodeArray[])     //函数名称和参量和返回值类型。
* 变量范围:  数组UniqueCodeArray[]作为函数形参，功能等同于指针，表示数组首元素的地址。
*            实际调用时该变量为数组元素名或者数组首元素的地址，且该数组元素个数>=6，唯一码读取结果存放在数组UniqueCodeArray[]中。
*            
* 返回值说明:  bool8, 读取唯一码成功返回TRUE, 否则返回FALSE
*
* 功能介绍:  读取加密芯片的6字节唯一码
*
* 关键备注： 1、用于激活仪表时， 调用者调用此函数获取唯一码，把唯一码发送给激活器
*               读取到的唯一码直接存放在UniqueCodeArray[]中
*            2、24AA02E48T的唯一码地址0xFA~0xFF
*               24AA02E48T memory: standard eeprom(00h~80h), write-protected node address block(80h~FFh)
*
* 未来提高:
* 
*/ 
bool8	ReadUniqueCode(uchar8  UniqueCodeArray[])
{
	uchar8  i;                           
	uchar8  SuccessfulRead = FALSE;    /* 读取成功，0表示读取失败，1表示读取成功	*/     
                                                                       
	SuccessfulRead = IICROM_LoadCharArrayData(UNIQUE_CODE_SAVE_ADDRESS, UniqueCodeArray, UNIQUE_CODE_BYTE_NUM); // Read 24AA02E48 ID(6 byte)
	if (SuccessfulRead == FALSE)
	{		  
		return FALSE;
	}
	  
	for (i = 0; i < UNIQUE_CODE_BYTE_NUM; i++)  /* 唯一码字节个数为6 */ 
	{
		if (UniqueCodeArray[i] != 0)           /* 唯一码不全为0 */ 
		{
			i = READ_CORRECT_UNIQUECODE_FLAG;
			break;
		}			   
	}
      
	if (i >= READ_CORRECT_UNIQUECODE_FLAG)
	{
		return TRUE;
	}
	else
	{
		return FALSE;	 
	}
}



/** bool8   CheckCalculationCodeForActivation(uchar8  LocalUniqueCode[],uchar8  LocalCalcCode[])     //函数名称和参量和返回值类型。
* 变量范围:  数组作为函数形参，功能等同于指针，表示数组首元素的地址。实际调用时该变量为数组元素名或者数组首元素的地址
*            LocalUniqueCode[]：加密芯片唯一码，唯一码有6字节，该数组元素个数>=6
*            LocalCalcCode[]：加密芯片的计算码，计算码有8字节，该数组元素个数>=8
*
* 返回值说明:  bool8，FALSE表示输入的唯一码与通过计算码反算出的唯一码不一致，TRUE表示两者是一致的
*
* 功能介绍:  检查为激活仪表所用的计算码
*
* 关键备注：
*
* 未来提高: 
* 
*/ 
bool8   CheckCalculationCodeForActivation(uchar8  LocalUniqueCode[], uchar8  LocalCalcCode[])
{
	uchar8  i;  
	uchar8  j;  	 
	uchar8  CalculatedUniqueCode[UNIQUE_CODE_BYTE_NUM] = {0}; /* 计算出的唯一码 */   
	  
      
	  /**********************通过计算码反算出6字节唯一码**********************/      
	for (i = 0; i < UNIQUE_CODE_BYTE_NUM; i++)      /* 加密芯片唯一码，唯一码有6字节 */ 
	{
		CalculatedUniqueCode[i] =  LocalCalcCode[i] - FIXED_VALUE;
		CalculatedUniqueCode[i] =  ~CalculatedUniqueCode[i]; 		       	
	}  
	  
	 /**********************通过计算码计算出的6字节唯一码与输入的唯一码对比**********************/
	for (j = 0; j < UNIQUE_CODE_BYTE_NUM; j++)      /* 加密芯片唯一码，唯一码有6字节 */ 
	{
		if (LocalUniqueCode[j] != CalculatedUniqueCode[j]) /* 输入的唯一码与通过计算码计算出的唯一码是否一样 */ 
		{		 
			return FALSE;
		}
		else
		{
			;
		}
	}   
	return TRUE;      
}



/** bool8  CheckCompanyNameAndCopyRightDeclareInProgram(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 检查对比一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查程序中的公司名称
*
* 关键备注：为了防止 盗版人员在保证crc不变的前提下进行修改局部内容，我们可以把内容分段在进行crc校验，
*           可以不规则的把版权声明进行分段，然后每段再进行一次crc。这里分成两段（前25字节，后34字节）
*
* 未来提高:
* 
*/ 
bool8	CheckCompanyNameInProgram(void)
{
	uchar8  i;                           
	uint16_t TempParaCRC1,TempParaCRC2;
	
	/********************************将程序中的公司名称进行每字节的对比**************************/
	for (i = 0; i < sizeof(G_CompanyName_RAM); i++)  
	{
		if (G_CompanyName_RAM[i] != G_CompanyName[i])
		{		
			return FALSE;
		}
		else
		{
			;
		}								 
	}
	
	/***********************将程序中的公司名称进行CRC校验，分两段检验**************************/    	
	/* 公司名称第一段CRC校验 */
	TempParaCRC1 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(G_CompanyName_RAM),COMPANY_NAME_CRC_BYTE1);
	if (TempParaCRC1 != COMPANY_NAME_CRC1)
	{
		return  FALSE;
	}
	else
	{
		;
	}
		
	/* 公司名称第二段CRC校验 */
	TempParaCRC2 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(&G_CompanyName_RAM[COMPANY_NAME_CRC_BYTE1]),sizeof(G_CompanyName_RAM)-COMPANY_NAME_CRC_BYTE1);
	if (TempParaCRC2 != COMPANY_NAME_CRC2)
	{
		return  FALSE;
	}
	else
	{
		;
	}
	return TRUE;
}


/** bool8  CheckCopyRightDeclareInProgram(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 检查对比一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查程序中的版权声明
*
* 关键备注：为了防止 盗版人员在保证crc不变的前提下进行修改局部内容，我们可以把内容分段在进行crc校验，
*           可以不规则的把版权声明进行分段，然后每段再进行一次crc. 这里分成三段（前50，中间100，后167）。
*
* 未来提高:
* 
*/ 
bool8	CheckCopyRightDeclareInProgram(void)
{
	uint16  i;   
	uchar8  CopyRightDeclare_Ref[10] = {0};  /* 版权声明中的关键字存放数组 */
	uint16_t TempParaCRC1,TempParaCRC2,TempParaCRC3;	
	
	/***********************将程序中的版权声明进行关键词检查**********************/
	CopyRightDeclare_Ref[0] = 'C';	
	CopyRightDeclare_Ref[1] = 'O';
	CopyRightDeclare_Ref[2] = 'N';	
	CopyRightDeclare_Ref[3] = 'N';
	CopyRightDeclare_Ref[4] = 'E';	
	CopyRightDeclare_Ref[5] = 'T';
	CopyRightDeclare_Ref[6] = 'E';	
	CopyRightDeclare_Ref[7] = 'C';
	CopyRightDeclare_Ref[8] = 'H';	
	CopyRightDeclare_Ref[9] = '(';
	for (i = COPYRIGHT_DECLARE_KEYWORD_START; i < COPYRIGHT_DECLARE_KEYWORD_END; i++)  
	{
		if (G_CopyRightDeclare[i] != CopyRightDeclare_Ref[i - COPYRIGHT_DECLARE_KEYWORD_START])
		{
		    return FALSE;
		}
	}
	 
	/***********************将程序中的版权声明进行CRC校验**********************/	
	/* 版权声明第一段CRC校验 */
	TempParaCRC1 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(G_CopyRightDeclare),COPYRIGHT_DECLARE_CRC_BYTE1);
	if (TempParaCRC1 != COPYRIGHT_DECLARE_CRC1)
	{
		return  FALSE;
	}
	else
	{
		;
	}  
	
	/* 版权声明第二段CRC校验 */
	TempParaCRC2 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(&G_CopyRightDeclare[COPYRIGHT_DECLARE_CRC_BYTE1]),COPYRIGHT_DECLARE_CRC_BYTE2-COPYRIGHT_DECLARE_CRC_BYTE1);
	if (TempParaCRC2 != COPYRIGHT_DECLARE_CRC2)
	{
	    return  FALSE;
	}
	else
	{
		;
	}   
	
	/* 版权声明第二段CRC校验 */
	TempParaCRC3 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(&G_CopyRightDeclare[COPYRIGHT_DECLARE_CRC_BYTE2]),sizeof(G_CopyRightDeclare)-COPYRIGHT_DECLARE_CRC_BYTE2);
	if (TempParaCRC3 != COPYRIGHT_DECLARE_CRC3)
	{
	    return  FALSE;
	}
	else
	{
		;
	}    	 	  	
        
	return TRUE;
}



/** bool8  ReadCompanyNameFromEEProm( )   //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 成功的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  读取存储在EEProm中的公司名称
*
* 关键备注：eeprom的读取，会比较耗时，所有超过1mS的工作可能都需要分成多次来完成。
*
* 未来提高: 
* 
*/ 
bool8	ReadCompanyNameFromEEProm(uint16_t COMPANYNAME_SAVE_ADDRESS,uint16_t CompanyNameReadByteNum)
{
	uchar8  i;  	
	bool8   ReadSuccess = TRUE;  /* 读取成功标志位 */			
//	bool8   SaveSuccess = FALSE; /* 存储成功标志位 */		
	uchar8  ReadData[AT24C64_PAGE_BYTE_NUM];	     /* 读取到的数据 */		
		
	ReadEeprom(COMPANYNAME_SAVE_ADDRESS+CompanyNameReadByteNum,ReadData,AT24C64_PAGE_BYTE_NUM); 
	for (i = 0; i < AT24C64_PAGE_BYTE_NUM; i++)  //check CompanyName
	{  
		if ((i+CompanyNameReadByteNum) < sizeof(G_CompanyName))
		{
			if (ReadData[i] != G_CompanyName[i+CompanyNameReadByteNum])
			{
				ReadSuccess = FALSE;
				WriteEeprom(COMPANYNAME_SAVE_ADDRESS + CompanyNameReadByteNum, (uint8_t *)(G_CompanyName+CompanyNameReadByteNum),AT24C64_PAGE_BYTE_NUM);
				break;
			}
			else
			{
				;					 			 
			}
		}
		else
		{
			break;
		}
	}
	if (ReadSuccess == FALSE)
	{
		return FALSE;     
	}
	else
	{
		return TRUE;
	}
}

/** bool8  CheckPartialCompanyNameInEEProm(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查存储在EEProm中的公司名称
*
* 关键备注：eeprom的读取，会比较耗时，所有超过1mS的工作可能都需要分成多次来完成。
*
* 未来提高: 
* 
*/ 
bool8	CheckPartialCompanyNameInEEProm(uint16_t COMPANYNAME_SAVE_ADDRESS,uint16_t *CompanyNameReadByteNum)
{	    
	bool8  ReadCompanyNameSuccess = FALSE;  /* 读取成功标志位 */	
	G_AllActivateCodeCheckFinish = FALSE;	    
	/***********************检查EEProm中公司名称**********************/
	if ( *CompanyNameReadByteNum < sizeof(G_CompanyName) )
	{
		ReadCompanyNameSuccess = ReadCompanyNameFromEEProm(COMPANYNAME_SAVE_ADDRESS,*CompanyNameReadByteNum);
		*CompanyNameReadByteNum = *CompanyNameReadByteNum+AT24C64_PAGE_BYTE_NUM;
		if (*CompanyNameReadByteNum > sizeof(G_CompanyName))
		{
			*CompanyNameReadByteNum = sizeof(G_CompanyName);
		}
	}
		
	if (ReadCompanyNameSuccess == TRUE)
	{	
		return TRUE;
	}
	else
	{
		return FALSE;  
	}   
}

/** bool8  ReadAllCompanyNameFromEEProm(uint16 SaveAddress)   //函数名称和参量和返回值类型。
* 变量范围:  SaveAddress为存储地址，0 ≤ SaveAddress ≤ 8191
*
* 返回值说明: bool8, 成功的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  读取存储在EEProm中的第一份公司名称全称
*
* 关键备注：当激活器端询问仪表激活码状态时，将存储在EEPROM中的公司名称一次性全读出来
*
* 未来提高: 
* 
*/ 
bool8	ReadAllCompanyNameFromEEProm(uint16 SaveAddress)
{
	bool8  ReadCompanyNameSuccess = FALSE;  /* 读取成功标志位 */
	uint16 CompanyNameReadByteNum = 0;
	G_AllActivateCodeCheckFinish = FALSE;	    
	/***********************检查EEProm中公司名称**********************/
	while (CompanyNameReadByteNum < sizeof(G_CompanyName) )
	{
		ReadCompanyNameSuccess = ReadCompanyNameFromEEProm(SaveAddress,CompanyNameReadByteNum);
		CompanyNameReadByteNum += AT24C64_PAGE_BYTE_NUM;
		if (CompanyNameReadByteNum > sizeof(G_CompanyName))
		{
			CompanyNameReadByteNum = sizeof(G_CompanyName);
		}
	}
		
	if (ReadCompanyNameSuccess == TRUE)
	{	
		return TRUE;
	}
	else
	{
		return FALSE;  
	}   
}



/** bool8  CheckAllCompanyNameInEEProm(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查存储在EEProm中的公司名称全称
*
* 关键备注：当激活器端询问仪表激活码状态时，将存储在EEPROM中的公司名称一次性全读出来
*
* 未来提高: 
* 
*/ 
bool8	CheckAllCompanyNameInEEProm(void)
{	    
	bool8  ReadCompanyName1Success = FALSE;  /* 读取成功标志位 */
	bool8  ReadCompanyName2Success = FALSE;  /* 读取成功标志位 */
	
	/***********************检查EEProm中第一份公司名称**********************/
	ReadCompanyName1Success = ReadAllCompanyNameFromEEProm(COMPANYNAME_SAVE_ADDRESS1);   
	/***********************检查EEProm中第二份公司名称**********************/
		   
	ReadCompanyName2Success = ReadAllCompanyNameFromEEProm(COMPANYNAME_SAVE_ADDRESS2); 
	if ( (ReadCompanyName1Success == TRUE) && (ReadCompanyName2Success == TRUE) )
	{	
		return TRUE;
	}	
	else
	{
		return FALSE; 
	}   
}


/** bool8  ReadCopyRightDeclareFromEEProm( )   //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 成功的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  读取存储在EEProm中的版权声明
*
* 关键备注：eeprom的读取，会比较耗时，所有超过1mS的工作可能都需要分成多次来完成。
*
* 未来提高: 
* 
*/ 
bool8	ReadCopyRightDeclareFromEEProm()
{
	uchar8  i;  	
	bool8   ReadSuccess = TRUE;  /* 读取成功标志位 */			
//	bool8   SaveSuccess = FALSE; /* 存储成功标志位 */		
	uchar8  ReadData[AT24C64_PAGE_BYTE_NUM];	     /* 读取到的数据 */		
		
	ReadEeprom(COPYRIGHT_SAVE_ADDRESS+G_CopyRightReadByteNum,ReadData,AT24C64_PAGE_BYTE_NUM); 
	for (i = 0; i < AT24C64_PAGE_BYTE_NUM; i++)  //check CopyRightDeclare
	{  
		if ((i+G_CopyRightReadByteNum) < sizeof(G_CopyRightDeclare))
		{
			if (ReadData[i] != G_CopyRightDeclare[i+G_CopyRightReadByteNum])
			{
				ReadSuccess = FALSE;
				WriteEeprom(COPYRIGHT_SAVE_ADDRESS + G_CopyRightReadByteNum, (uint8_t*)(G_CopyRightDeclare+G_CopyRightReadByteNum),AT24C64_PAGE_BYTE_NUM);
				break;
			}
			else
			{
				;					 			 
			}
		}
		else
		{
			break;
		}
	}

	if (ReadSuccess == FALSE)
	{		
		return FALSE;   
	}
	else
	{
		return TRUE;
	}
}


/** bool8  CheckPartialCopyRightDeclareInEEProm(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查存放在EEProm中的部分版权声明
*
* 关键备注：eeprom的读取，会比较耗时，所有超过1mS的工作可能都需要分成多次来完成。
*
* 未来提高: 
* 
*/ 
bool8	CheckPartialCopyRightDeclareInEEProm(void)
{
	bool8  ReadCopyRightSuccess = FALSE;  /* 读取成功标志位 */
	
       /***********************检查EEProm中版权声明**********************/	    	   
	if ( G_CopyRightReadByteNum < sizeof(G_CopyRightDeclare) )
	{			    
		ReadCopyRightSuccess = ReadCopyRightDeclareFromEEProm();
		G_CopyRightReadByteNum += AT24C64_PAGE_BYTE_NUM;
		if (G_CopyRightReadByteNum > sizeof(G_CopyRightDeclare))
		{
			G_CopyRightReadByteNum = 0;	
			G_CompanyNameReadByteNum1 = 0;
			G_CompanyNameReadByteNum2 = 0;
			G_AllActivateCodeCheckFinish = TRUE;				
		}
	}	
		
	if (ReadCopyRightSuccess == TRUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE; 
	}  
}



/** bool8  ReadAllCopyRightDeclareFromEEProm( )   //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 成功的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  读取存储在EEProm中的版权声明全称
*
* 关键备注：当激活器端询问仪表激活码状态时，将存储在EEPROM中的版权声明一次性全读出来
*
* 未来提高: 
* 
*/ 
bool8	ReadAllCopyRightDeclareFromEEProm()
{
	uint16  i;
	uchar8 TempC;
	bool8   ReadSuccess = TRUE;  /* 读取成功标志位 */	
//	bool8   SaveSuccess = FALSE;  /* 存储成功标志位	*/	
	uchar8  ReadData[AT24C64_PAGE_BYTE_NUM];	     /* 读取到的数据 */		
		
	for (i = 0; i < sizeof(G_CopyRightDeclare); i = i+AT24C64_PAGE_BYTE_NUM)  
	{ 			 
		ReadEeprom(COPYRIGHT_SAVE_ADDRESS+i,ReadData,AT24C64_PAGE_BYTE_NUM);
		for (TempC = 0; TempC < AT24C64_PAGE_BYTE_NUM; TempC++)  //check CompanyName
		{  
			if ((TempC+i) < sizeof(G_CopyRightDeclare))
			{
				if (ReadData[TempC] != G_CopyRightDeclare[i+TempC])
				{
					ReadSuccess = FALSE;
					WriteEeprom(COPYRIGHT_SAVE_ADDRESS + i, (uint8_t*)(G_CopyRightDeclare+i),AT24C64_PAGE_BYTE_NUM);
					break;
				}
				else
				{
					;					 			 
				}
			}
			else
			{
				break;
			}
		}
	}
	if (ReadSuccess == FALSE)
	{		
		return FALSE;   
	}
	else
	{
		 return TRUE;
	}
}




/** bool8  CheckAllCopyRightDeclareInEEProm(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查存放在EEProm中的版权声明全称
*
* 关键备注：当激活器端询问仪表激活码状态时，将存储在EEPROM中的版权声明一次性全读出来
*
* 未来提高: 
* 
*/ 
bool8	CheckAllCopyRightDeclareInEEProm(void)
{
	bool8  ReadCopyRightSuccess = FALSE;  /* 读取成功标志位 */
	   
	   	
       /***********************检查EEProm中版权声明**********************/
	    	   			    
	ReadCopyRightSuccess = ReadAllCopyRightDeclareFromEEProm();	   
	
	if (ReadCopyRightSuccess == TRUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;  
	} 
}



/** bool8  CheckCompanyNameAndCopyRightDeclare(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 检查对比一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查公司名称和版权声明
*            公司名称和版权声明的检查分为两种: 通过G_EnableActivatorAskState来区分
*            1、正常的检查: 分时分段检查，即每隔一定的时间进行激活码的检查，由于激活码较多
*               若平时一次性全部检查一遍的话，负荷较大，因此每执行一次该函数就检查其中的一段
*               激活码，直至激活码全部检查一遍，再将计时变量清零，如此循环。
*            2、可通过使G_EnableActivatorAskState = 1，使得所有激活码一次全部被检查一遍
*               常用于通过液晶端获取仪表的激活码正确与否，或者初始上电时将所有激活码全部检查一遍
*               看仪表的激活码正确与否，并将其结果标识显示在液晶端
*
* 关键备注：
*
* 未来提高:
* 
*/ 
bool8	CheckCompanyNameAndCopyRightDeclare(void)
{ 
	bool8  CompanyNameCorrect1 = FALSE;
	bool8  CompanyNameCorrect2 = FALSE;
	bool8  CompanyNameCorrect3 = FALSE;
	bool8  CopyRightCorrect1 = FALSE;
	bool8  CopyRightCorrect2 = FALSE;
	   
	if (G_EnableActivatorAskState == FALSE)  /* 正常检查激活码: 分时分段检查*/
	{
//		TestTime_Start = HAL_GetTick();
		CompanyNameCorrect1 = CheckCompanyNameInProgram();
		CopyRightCorrect1 = CheckCopyRightDeclareInProgram();
		
		if ( (G_CopyRightReadByteNum == 0) && (G_CompanyNameReadByteNum2 == 0) && (G_CompanyNameReadByteNum1 < sizeof(G_CompanyName)) )
		{
			CompanyNameCorrect2 = CheckPartialCompanyNameInEEProm(COMPANYNAME_SAVE_ADDRESS1,&G_CompanyNameReadByteNum1);
		}
		else if ( (G_CopyRightReadByteNum == 0) && (G_CompanyNameReadByteNum1 == sizeof(G_CompanyName)) && (G_CompanyNameReadByteNum2 < sizeof(G_CompanyName)) )
		{
			CompanyNameCorrect3 = CheckPartialCompanyNameInEEProm(COMPANYNAME_SAVE_ADDRESS2,&G_CompanyNameReadByteNum2);
		}
		else if ( (G_CompanyNameReadByteNum1 == sizeof(G_CompanyName)) && (G_CompanyNameReadByteNum2 == sizeof(G_CompanyName))&& (G_CopyRightReadByteNum < sizeof(G_CopyRightDeclare)) )
		{
			CopyRightCorrect2 = CheckPartialCopyRightDeclareInEEProm();
		}
	   
//		TestTime_End = HAL_GetTick();
		if ( (CompanyNameCorrect1 == TRUE) && (CopyRightCorrect1 == TRUE) && 
			((CompanyNameCorrect2 == TRUE)|| (CompanyNameCorrect3 == TRUE)||(CopyRightCorrect2 == TRUE)) )
		{
			return TRUE;
		}
		else
		{			 
			return FALSE;
		}
	}
	else  /* 随时查询激活码，一次性将所有激活码检查一遍 */
	{
//		TestTime_Start = HAL_GetTick();
		CompanyNameCorrect1 = CheckCompanyNameInProgram();
		CopyRightCorrect1 = CheckCopyRightDeclareInProgram();
		CompanyNameCorrect2 = CheckAllCompanyNameInEEProm();
		CopyRightCorrect2 = CheckAllCopyRightDeclareInEEProm();
//	   	TestTime_End = HAL_GetTick();
		if ( (CompanyNameCorrect1 == TRUE) && (CopyRightCorrect1 == TRUE) && 
			(CompanyNameCorrect2 == TRUE) && (CopyRightCorrect2 == TRUE) )
		{
			return TRUE;
		}
		else
		{		
			return FALSE;
		}
	}
}


/** bool8  CheckValidityOfCalculationCode(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 计算码合法返回TRUE，否则返回FALSE
*
* 功能介绍:  检查计算码激活合法性
*
* 关键备注：调用该函数的前提是已经从加密芯片中读取出计算码
*
* 未来提高:
* 
*/ 
bool8	CheckValidityOfCalculationCode(void)
{
	bool8  ReadSuccess = FALSE;  /* 成功读取唯一码，TRUE代表成功， FALSE代表失败 */
	bool8  CalcCodeCompareResult = FALSE;  /* 计算码比较结果，TRUE代表成功， FALSE代表失败 */	  
	  
	ReadSuccess = ReadUniqueCode(G_UniqueCode);
	if (ReadSuccess == TRUE) 
	{ 
		CalcCodeCompareResult = CheckCalculationCodeForActivation(G_UniqueCode, G_CalcSourceCode); 
		if (CalcCodeCompareResult == TRUE) 
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



/** bool8  CheckCalculationCodeAndReverseCodeAndFixedCode(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, 读取到的计算码（原码、其反码以及原码+固定码）>=2种一致的话，返回TRUE，否则返回FALSE
*
* 功能介绍:  检查计算码原码、其反码以及原码+固定码
*
* 关键备注：3份不同版本，如果3份都是对的，认为激活码正确。
*                        如果2份对的，一份错误。认为激活码正确，并修正错误的数据。
*                        如果2份错误或者3份都错误，则认为失败，不修复，启动（报复行为）
*
* 未来提高:
* 
*/ 
bool8	CheckCalculationCodeAndReverseCodeAndFixedCode(void)
{
	uchar8  i;  
	uchar8  j;  
	uchar8  k;  
	  
	bool8   ReadSourceCodeSuccess = FALSE;   /* 读取原码成功标志位 */
	bool8   ReadReverseCodeSuccess = FALSE;  /* 读取反码成功标志位 */
	bool8   ReadFixedCodeSuccess = FALSE;    /* 读取固定码成功标志位 */
	  
	bool8   SourceReverseCodeCheckResult = FALSE;  /* 原码、反码比较结果 */
	bool8   SourceFixedCodeCheckResult = FALSE;    /* 原码、固定码比较结果 */
	bool8   ReverseFixedCodeCheckResult = FALSE;   /* 反码、固定码比较结果 */
	  
	uchar8  CalcCodeErrorCount = 0;          /* 错误计算码计数 */
	  
	uchar8  ReverseCode_Source[CALC_CODE_BYTE_NUM] = {0};   /* 由原码计算出的反码 */
	uchar8  FixedCode_Source[FIXED_CODE_BYTE_NUM] = {0};    /* 由原码计算出的固定码 */
	uchar8  ReverseCode_Fixed[CALC_CODE_BYTE_NUM] = {0};    /* 由固定码计算出的反码 */
      
	ReadSourceCodeSuccess = IICROM_LoadCharArrayData(SOURCE_CODE_SAVE_ADDRESS, G_CalcSourceCode, CALC_CODE_BYTE_NUM);
	if (ReadSourceCodeSuccess == FALSE)
	{
		return FALSE;     
	} 
			  
	ReadReverseCodeSuccess = IICROM_LoadCharArrayData(REVERSE_CODE_SAVE_ADDRESS,G_CalcReverseCode, CALC_CODE_BYTE_NUM); //计算码反码比较
	if (ReadReverseCodeSuccess == FALSE)
	{
		return FALSE;
	}
		  
	ReadFixedCodeSuccess = IICROM_LoadCharArrayData(FIXED_CODE_SAVE_ADDRESS, G_CalcFixedCode, FIXED_CODE_BYTE_NUM); //计算码原码+固定码比较
	if (ReadFixedCodeSuccess == FALSE)
	{
		return FALSE;	
	}
            
	for (i = 0; i < CALC_CODE_BYTE_NUM; i++)  /* 原码、反码比较 */
	{
		ReverseCode_Source[i] = ~G_CalcSourceCode[i];
		if (G_CalcReverseCode[i] != ReverseCode_Source[i])
		{
			SourceReverseCodeCheckResult = FALSE;
			CalcCodeErrorCount++;
			break;
		}	
	}      
	  
	for (j = 0; j < FIXED_CODE_BYTE_NUM; j++)  /* 原码、固定码比较 */
	{	
		if (j < 8)
		{		  
			FixedCode_Source[j] =  G_CalcSourceCode[j];
		}
		else
		{
			FixedCode_Source[FIXED_CODE_BYTE_NUM - 2] = FIXED_CODE1;
			FixedCode_Source[FIXED_CODE_BYTE_NUM - 1] = FIXED_CODE2;
		}
		      
		if (G_CalcFixedCode[j] != FixedCode_Source[j])
		{
			SourceFixedCodeCheckResult = FALSE;
			CalcCodeErrorCount++;
			break;
		}			 			   
	}
	  
	for (k = 0; k < CALC_CODE_BYTE_NUM; k++)  /* 反码、固定码比较 */
	{
		ReverseCode_Fixed[k] = ~G_CalcFixedCode[k];
		if (G_CalcReverseCode[k] != ReverseCode_Fixed[k])
		{
			ReverseFixedCodeCheckResult = FALSE;
			CalcCodeErrorCount++;
			break;
		}	
	}      
	    
	/***********************对检查结果进行分析处理*********************/
	if (CalcCodeErrorCount == 0)      /* 3份都是对的，认为激活码正确 */
	{
		return TRUE;
	}
	else if (CalcCodeErrorCount == 2)   /* 三组对比中只有一组是对的，即3份码中2份是对的，认为激活码正确，并修正错误的数据。*/
	{
		if ( (SourceReverseCodeCheckResult  == FALSE) && (SourceFixedCodeCheckResult == FALSE) )  /* 原码与反码、原码与固定码比较均不对*/
		{				  
			IICROM_SaveCharArrayData(SOURCE_CODE_SAVE_ADDRESS, G_CalcFixedCode, CALC_CODE_BYTE_NUM);  /* save calculation source code( 8 byte )  */                
		}
		else if ( (SourceReverseCodeCheckResult  == FALSE) && (ReverseFixedCodeCheckResult == FALSE) )  /* 原码与反码、反码与固定码比较均不对*/ 
		{
			IICROM_SaveCharArrayData(REVERSE_CODE_SAVE_ADDRESS, ReverseCode_Source, CALC_CODE_BYTE_NUM);  /* save calculation reverse code( 8 byte ) */                 
              
		}
		else if ( (SourceFixedCodeCheckResult  == FALSE) && (ReverseFixedCodeCheckResult == FALSE) )  /* 原码与固定码、反码与固定码比较均不对*/  
		{
			IICROM_SaveCharArrayData(FIXED_CODE_SAVE_ADDRESS, FixedCode_Source, FIXED_CODE_BYTE_NUM); /* save calculation fixed code( 10 byte ) */                   
             
		}		
		return TRUE;
	}	
	else 		/* 3份码中如果2份错误或者3份都错误，则认为失败，不修复 */
	{
		return FALSE;
	}     
}


/** bool8   CheckCorrectnessOfEncryptionChip(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8， TRUE代表加密芯片正确，FALSE代表加密芯片不正确
*
* 功能介绍:  检查加密芯片的正确性
*
* 关键备注：加密芯片的80h---FFh为写保护区，往该存储区的任一地址写入数据，再读取出来，写入的和读取的是不一样的
*
* 未来提高: 
* 
*/ 
bool8	CheckCorrectnessOfEncryptionChip(void)
{
	uchar8  WriteData[AA02E48_PAGE_BYTE_NUM] = {0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80};  /* 待写数据 */
	bool8   SaveSuccess = FALSE;  /* 存储成功标志位 */
	SaveSuccess = IICROM_SaveCharArrayData(WRITE_PROTECT_SAVE_ADDRESS,WriteData,AA02E48_PAGE_BYTE_NUM);
	if (SaveSuccess == TRUE)  /* 加密芯片写保护区是存储不了的，如果存储成功，证明不是加密芯片 */
	{
		RecordPerfToFRAM(WRITEENCRYPTIONCHIPERRORCOUNT);
		return FALSE;
	}
	else
	{
		return TRUE;	   
	}
}


/** bool8   CheckCalculationCode(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8， TRUE代表计算码正确，FALSE代表计算码不正确
*
* 功能介绍:  检查计算码, 包括加密芯片的正确性、计算码的正确性以及计算码激活合法性。
*
* 关键备注：
*
* 未来提高: 
* 
*/ 
bool8	CheckCalculationCode(void)
{
	bool8  EncryptionchipCorrect = FALSE;
	bool8  CalcCodeCorrect = FALSE;
	bool8  CalcCodeValid = FALSE;
	   
	EncryptionchipCorrect = CheckCorrectnessOfEncryptionChip();
	CalcCodeCorrect = CheckCalculationCodeAndReverseCodeAndFixedCode();
	CalcCodeValid = CheckValidityOfCalculationCode();
	   
	if ( (EncryptionchipCorrect == TRUE) &&(CalcCodeCorrect == TRUE) && (CalcCodeValid == TRUE) )
	{
		return  TRUE;
	}
	else
	{		  
		return  FALSE;
	}
}



/** bool8   CheckActivationCode(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8， TRUE代表激活码正确，FALSE代表激活码不正确
*             返回值的使用: 在初始上电把所有激活码检查一遍时，使用该bool8型返回值
*             在主函数while(1)中用到的是激活码检查失败次数G_ActivateCodeErrorCount，根据G_ActivateCodeErrorCount
*             的大小判断是否进入报复行为状态
*
* 功能介绍:  检查激活码, 约CHECK_ACTIVATIONCODE_TIME(1800,半小时)检查一次，累计检查失败的次数，
*            失败次数最大值等于CHECK_ACTIVATIONCODE_FAIL_NUM(10次)
*            具体定时时间CHECK_ACTIVATIONCODE_TIME以及失败次数上限CHECK_ACTIVATIONCODE_FAIL_NUM可由调用者自己设定
*
* 关键备注：激活码的检查根据G_EnableActivatorAskState的值分为两种工作状态: 
*           (1)激活定时检查: 低负荷运行(把整体检查的工作量平均分配到每一次程序调用中)，放主循环中执行  
*           (2)只在询问激活状态时检查: 全部检查一遍，目前只是在初始上电时以及激活器端询问激活状态时才执行这种工作状态
*
* 未来提高: 
* 
*/ 
bool8	CheckActivationCode(void)
{   
	bool8  CalcCodeCorrect = FALSE;          /* “计算码正确”标志位 */
	bool8  NameAndCopyRightCorrect = FALSE;  /* “公司名称和版权声明正确”标志位 */
//	bool8  SaveSuccess = FALSE;
	    
	if (G_EnableActivatorAskState == FALSE)
	{
		if(G_CheckActivateCodeTimer >= CHECK_ACTIVATIONCODE_TIME)   /* CHECK_ACTIVATIONCODE_TIME=1800s */    
		{          
			NameAndCopyRightCorrect = CheckCompanyNameAndCopyRightDeclare();
			   
			if (G_AllActivateCodeCheckFinish == TRUE)
			{
				RecordPerfToFRAM(CHECKACTIVETOTALCOUNT);
				G_CheckActivateCodeTimer = 0;
				G_AllActivateCodeCheckFinish = FALSE;				   
			   
				CalcCodeCorrect = CheckCalculationCode();
				if ((CalcCodeCorrect == FALSE) || (NameAndCopyRightCorrect == FALSE))
				{
					G_AllEEpromDataCheckSuccess = FALSE;
				}

				if (G_AllEEpromDataCheckSuccess == TRUE) 
				{
					G_ActivateCodeErrorCount = 0;
					return  TRUE;
				}
				else
				{
					RecordPerfToFRAM(CHECKACTIVEERRORCOUNT);
					G_AllEEpromDataCheckSuccess = TRUE;
					if (G_ActivateCodeErrorCount < CHECK_ACTIVATIONCODE_FAIL_NUM)
					{
						G_ActivateCodeErrorCount++;
					}
					return  FALSE;
				}
			}
			else
			{
				return  FALSE;	
			}		       
		}
		else
		{
			return  FALSE;	
		}
	}
	else
	{	
		RecordPerfToFRAM(CHECKACTIVETOTALCOUNT);
		CalcCodeCorrect = CheckCalculationCode();
		NameAndCopyRightCorrect = CheckCompanyNameAndCopyRightDeclare();
		G_EnableActivatorAskState = FALSE; 		   
		  
		if ( (CalcCodeCorrect == TRUE) && (NameAndCopyRightCorrect == TRUE) )
		{
			G_ActivateCodeErrorCount = 0;
			return  TRUE;
		}
		else
		{	
			RecordPerfToFRAM(CHECKACTIVEERRORCOUNT);
			G_ActivateCodeErrorCount = 1;
			return  FALSE;
		}		   
	}
}


/** bool8  RevengeOrNot(void)    //函数名称和参量和返回值类型。
* 变量范围:  
*
* 返回值说明: bool8, FALSE表示不实施报复行为，TRUE表示实施报复行为
*
* 功能介绍:  是否实施报复行为。（报复行为：非正常工作模式，具体可由被调用者自己决定）
*
* 关键备注：
*
* 未来提高:
* 
*/ 
bool8	RevengeOrNot(void)
{
	if (G_ActivateCodeErrorCount < CHECK_ACTIVATIONCODE_FAIL_NUM) 
	{           
		return FALSE;
	}
	else 
	{	
		return TRUE;
	}      
}



/** bool8  SaveCalcCodeToEncryptionChip(void)     //函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明:  bool8，保存成功返回TRUE，否则返回FALSE 
*
* 功能介绍: 将计算码（原码、反码以及固定码(原码+0xA5+0x5A)）存储在加密芯片24AA02E48T中Standard EEPROM中           
*
* 关键备注：24AA02E48T中0x00~0x80为标准EEPROM区域，0x80~0xFF为24AAE0248T唯一码存储区（写保护）
*           Page Write Time 3 ms, typical
*           Page Write Buffer:  8-byte page (24AA02E48/24AA02E64)  
*
* 未来提高:
* 
*/
bool8	SaveCalcCodeToEncryptionChip(void)
{
	uchar8  i;            
	uchar8  j;             
	bool8   CalcSourceCodeSuccessfulSave = FALSE;    /* 计算码原码成功保存，TRUE表示保存成功，FALSE 表示保存失败 */
	bool8   CalcReverseCodeSuccessfulSave = FALSE;	 /* 计算码反码成功保存，TRUE表示保存成功，FALSE 表示保存失败 */
	bool8   CalcFixedCodeSuccessfulSave = FALSE;     /* 计算码固定码成功保存，TRUE表示保存成功，FALSE 表示保存失败 */
	
	/* 保存计算码原码,8字节 */
	CalcSourceCodeSuccessfulSave = IICROM_SaveCharArrayData(SOURCE_CODE_SAVE_ADDRESS, G_CalcSourceCode, CALC_CODE_BYTE_NUM); 
	if (CalcSourceCodeSuccessfulSave == FALSE)
	{
	return FALSE;
	}
	
	/*  保存计算码反码,8字节 */		
	for (i = 0; i < CALC_CODE_BYTE_NUM; i++)                           
	{
	     G_CalcReverseCode[i] = ~G_CalcSourceCode[i];
	}		 
	CalcReverseCodeSuccessfulSave = IICROM_SaveCharArrayData(REVERSE_CODE_SAVE_ADDRESS, G_CalcReverseCode, CALC_CODE_BYTE_NUM);  
	if (CalcReverseCodeSuccessfulSave == FALSE)
	{
        return FALSE; 
	}
	
	/*  保存计算码固定码,10字节 */
	for (j = 0; j < CALC_CODE_BYTE_NUM; j++)                           
	{
         G_CalcFixedCode[j] = G_CalcSourceCode[j];
	}
	G_CalcFixedCode[FIXED_CODE_BYTE_NUM - 2] = FIXED_CODE1;
	G_CalcFixedCode[FIXED_CODE_BYTE_NUM - 1] = FIXED_CODE2;
	
	CalcFixedCodeSuccessfulSave = IICROM_SaveCharArrayData(FIXED_CODE_SAVE_ADDRESS, G_CalcFixedCode, FIXED_CODE_BYTE_NUM);
	if (CalcFixedCodeSuccessfulSave == FALSE)
	{
        return FALSE;
	}
			
	return TRUE;
}


/** uchar8  SaveCopyRightDeclare(void)     //函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: uchar8 ，保存成功返回1，否则返回0
*
* 功能介绍:  保存版权声明 
*
* 关键备注： 
*
* 未来提高:
* 
*/
bool8	SaveCopyRightDeclare(void)
{	    
	bool8   SaveSuccess = FALSE;  /* 保存成功标志位 */
		
	/************************存储版权声明************************/
	SaveSuccess = ReadAllCopyRightDeclareFromEEProm();
//	if(SaveSuccess == FALSE)
//	{
//		WriteEeprom(COPYRIGHT_SAVE_ADDRESS, (uint8_t *)(G_CopyRightDeclare),sizeof(G_CopyRightDeclare));
//		IICRomDelay(IICROM_DELAY_5MS);
//		SaveSuccess = ReadAllCopyRightDeclareFromEEProm();
//	}
	return (SaveSuccess);	
}



/** bool8  SaveCompanyNameToEEProm(uint16 SaveAddress)     // 函数名称和参量和返回值类型。
* 变量范围: SaveAddress为存储地址，0 ≤ SaveAddress ≤ 8191
*
* 返回值说明: bool8，保存成功返回TRUE，否则返回FALSE
*
* 功能介绍:  保存公司名称到EEPROM中
*
* 关键备注： 1、为保证存储的可靠性，公司名称保存两份，
*               因为加密芯片24AA02E48T的可写存储区容量为00H~7FH，共128字节，存储空间不够存储公司名称，因此将其存储在AT24C64中
*            2、AT24CXX写操作时，寻址设备（单片机）发送停止条件来终止写序列，此时，EEPROM进入到内部自计时写周期（最大5ms）
*               在写周期期间所有的输入不使能，且EEPROM在写操作完成后才会响应
*
* 未来提高:
* 
*/
bool8  SaveCompanyNameToEEProm(uint16 SaveAddress)
{	
	bool8   SaveCompanyNameSuccess = FALSE;          /* 成功保存公司名称，TRUE代表成功， FALSE代表失败 */		
	
	/* 按页存储数据 */
        if (SaveCompanyNameSuccess == FALSE)
	{
		return FALSE;	
	}
	return TRUE;
}


/** bool8  SaveCompanyName(void)     // 函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: bool8，保存成功返回TRUE，否则返回FALSE
*
* 功能介绍:  保存公司名称 
*
* 关键备注： 1、为保证存储的可靠性，公司名称保存两份，
*               因为加密芯片24AA02E48T的可写存储区容量为00H~7FH，共128字节，存储空间不够存储公司名称，因此将其存储在AT24C64中
*            2、AT24CXX写操作时，寻址设备（单片机）发送停止条件来终止写序列，此时，EEPROM进入到内部自计时写周期（最大5ms）
*               在写周期期间所有的输入不使能，且EEPROM在写操作完成后才会响应
*
* 未来提高:
* 
*/
bool8	SaveCompanyName(void)
{	   	
	bool8   SaveCompanyName1Success = FALSE;           /* 成功保存第一份公司名称，TRUE代表成功， FALSE代表失败 */
	bool8   SaveCompanyName2Success = FALSE;           /* 成功保存第二份公司名称，TRUE代表成功， FALSE代表失败 */ 
	   
		
	/***********************存储第一份公司名称*********************/
	WriteEeprom(COMPANYNAME_SAVE_ADDRESS1, (uint8_t *)(G_CompanyName),sizeof(G_CompanyName));
	SaveCompanyName1Success = ReadAllCompanyNameFromEEProm(COMPANYNAME_SAVE_ADDRESS1);
	if (SaveCompanyName1Success == FALSE)
	{
		return FALSE;
	}
	else
	{
		;
	}
		
	/**********************存储第二份公司名称**********************/
	WriteEeprom(COMPANYNAME_SAVE_ADDRESS2, (uint8_t *)(G_CompanyName),sizeof(G_CompanyName));
	SaveCompanyName2Success = ReadAllCompanyNameFromEEProm(COMPANYNAME_SAVE_ADDRESS2);
	if (SaveCompanyName2Success == FALSE)
	{
		return FALSE;
	}
	else
	{
		;
	}
			
	return TRUE;	   
}


/** bool8  SaveCalculationCode(void)     //函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: bool8，保存成功返回TRUE，否则返回FALSE
*
* 功能介绍:  保存计算码  
*
* 关键备注： 考虑到存储的可靠性，我们把计算码存储3份不同的版本。
*            版本1：计算码原码-------------------------------------8字节
*            版本2：计算码反码-------------------------------------8字节
*            版本3：计算码原码+固定码（0xA5,0x5A）-----------------10字节
*
* 未来提高:
* 
*/
bool8	SaveCalculationCode(void)
{
	bool8  ReadSuccess = FALSE;  /* 成功读取唯一码，TRUE代表成功， FALSE代表失败 */
	bool8  SaveSuccess = FALSE;  /* 成功保存计算码，TRUE代表成功， FALSE代表失败 */
	bool8  CalcCodeCompareResult = FALSE;  /* 计算码比较结果，TRUE代表成功， FALSE代表失败  */
	  
	ReadSuccess = ReadUniqueCode(G_UniqueCode);
	if (ReadSuccess == TRUE) 
	{ 
		CalcCodeCompareResult = CheckCalculationCodeForActivation(G_UniqueCode,G_CalcSourceCode); 
		if (CalcCodeCompareResult == TRUE) 
		{     
			SaveSuccess = SaveCalcCodeToEncryptionChip();
			if (SaveSuccess == TRUE)
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
	else
	{	   
		return FALSE;
	}
}


/** bool8  ActivateMeter(void)     //函数名称和参量和返回值类型。
* 变量范围: 
*
* 返回值说明: bool8 ，仪表激活成功返回TRUE，否则返回FALSE
*
* 功能介绍:  激活仪表
*            激活仪表的过程实际就是生成并存储激活码的过程，激活码包括由唯一码计算出的计算码原码、反码、原码+固定码、
*            公司名称、版权声明，这些激活码存储成功就代表仪表激活成功，否则就代表仪表激活失败
*
* 关键备注： 该函数用于使用激活器来激活仪表时，当仪表收到激活器发送来的计算码时再调用该函数
*
* 未来提高:
* 
*/
bool8	ActivateMeter(void)
{
	bool8  CalcCodeSuccessfulSave = FALSE;      /* 计算码成功保存，FALSE表示不成功，TRUE表示成功 */
	bool8  CompanyNameSuccessfulSave = FALSE;   /* 公司名称成功保存，FALSE表示不成功，TRUE表示成功 */
	bool8  CopyRightSuccessfulSave = FALSE;     /* 版权声明成功保存，FALSE表示不成功，TRUE表示成功 */
	  
	CalcCodeSuccessfulSave = SaveCalculationCode();
	CompanyNameSuccessfulSave = SaveCompanyName();
	CopyRightSuccessfulSave = ReadAllCopyRightDeclareFromEEProm();//SaveCopyRightDeclare();
	  
	if ( (CalcCodeSuccessfulSave == FALSE) || (CompanyNameSuccessfulSave == FALSE) || (CopyRightSuccessfulSave == FALSE) )
	{
		G_ActivateMeterSuccess = FALSE;
		return FALSE;
	}
	else
	{
		G_ActivateMeterSuccess = TRUE;
		return TRUE;
	} 	 
}

uchar8	Read_ActivateCodeErrorCount(void)
{
	return(G_ActivateCodeErrorCount);
}

void	IncCheckActivateCodeTimer(void)
{
	G_CheckActivateCodeTimer++;
}
