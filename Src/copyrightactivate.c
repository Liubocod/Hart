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



/****************************CopyRightActivate.c�ļ�˵��**********************************/
//���ļ�Ϊ�����Ʒ��غ���
/*************************************************************************************************/


#include "CopyRightActivate.h"
#include "ROM24AA02E48T.h"
#include "DataType.h"
#include "crc.h"
#include "perf_management.h"
#include "Eeprom.h"


#define  COMPANY_NAME       "CONNETECH Electronics Ltd(BeijingRuiDaYiBiaoYouXianGongSi)"    // sizeof(COMPANY_NAME)=59Byte
#define  COPYRIGHT_DECLARE  "All datas in this eeprom,flash or mcu is the important property of CONNETECH(BeijingRuiDaYiBiaoYouXianGongSi).The copyrights belong to connetech (BeijingRuiDaYiBiaoYouXianGongSi),and All rights reserved.To read out or copy these datas in eeprom,flash or mcu without permission of CONNETECH is Copyright infringement."   // sizeof(COPYRIGHT_DECLARE)=317Byte


#define  FIXED_CODE_BYTE_NUM       	        CALC_CODE_BYTE_NUM + 2     /* �̶����ֽ��� */

#define  UNIQUE_CODE_SAVE_ADDRESS           0xFA                       /* Ψһ��洢��ַFAh~FFh */
#define  READ_CORRECT_UNIQUECODE_FLAG       0xA8                       /* ��ȡ����ȷ��Ψһ���־�� */ 

#define  FIXED_CODE1                        0xA5   /* "������+�̶���" �еĹ̶��� */
#define  FIXED_CODE2                        0x5A   /* "������+�̶���" �еĹ̶��� */

#define  FIXED_VALUE                        0x01   /* �ɼ���������Ψһ��ʱ �㷨���õ��Ķ�ֵ */


uchar8    G_CompanyName_RAM[] = {COMPANY_NAME};              /* �����RAM�еĹ�˾���� */
uchar8    G_CopyRightDeclare_RAM[] = {COPYRIGHT_DECLARE};    /* �����RAM�еİ�Ȩ���� */

const uint8_t   G_CompanyName[] = {COMPANY_NAME};              /* ����ڳ����洢���еĹ�˾���� */
const uint8_t   G_CopyRightDeclare[] = {COPYRIGHT_DECLARE};    /* ����ڳ����洢���еİ�Ȩ���� */
  
uchar8    G_UniqueCode[UNIQUE_CODE_BYTE_NUM] = {0};         /* ����оƬΨһ�������� */
uchar8    G_CalcSourceCode[CALC_CODE_BYTE_NUM] = {0};       /* ����оƬ������ԭ�������� */
uchar8    G_CalcReverseCode[CALC_CODE_BYTE_NUM] = {0};      /* ����оƬ�����뷴�������� */   
uchar8    G_CalcFixedCode[FIXED_CODE_BYTE_NUM] = {0};       /* ����оƬ������̶��������� */ 

uint16_t    G_CompanyNameReadByteNum1 = 0;                    /* ��EEPROM�ж�ȡ�Ĺ�˾����1�ֽ��� */
uint16_t    G_CompanyNameReadByteNum2 = 0;                    /* ��EEPROM�ж�ȡ�Ĺ�˾����2�ֽ��� */
uint16    G_CopyRightReadByteNum = 0;                       /* ��EEPROM�ж�ȡ�İ�Ȩ�����ֽ��� */


uint16  G_CheckActivateCodeTimer = 0;     /* ���ڼ�鼤����(�����롢��˾���ơ���Ȩ����)�õļ�ʱ����,���ø��ļ�ʱ���ñ������ڶ�ʱ���ж��н���++
                                             �� >= CHECK_ACTIVATIONCODE_TIME ʱ����һ�μ������� */
uchar8  G_ActivateCodeErrorCount = 0;     /* ������(�����롢��˾���ơ���Ȩ����)�������,��鵽���������ʱ�ñ���+1
                                             �� >= CHECK_ACTIVATIONCODE_FAIL_NUMʱ���ñ�����Ϊ */

bool8   G_EnableActivatorAskState = FALSE;  /* ѯ�ʼ���״̬ʹ�ܣ���鼤����ʱ�����֣�һ����ϵͳ��ʱ��飬��������Ĺ�����ƽ�����䵽ÿһ�γ��������
                                               ����һ���ǵ��Ǳ�ļ���״̬��ѯ��ʱ�������м�����Ϣ���һ�� */

bool8   G_EnableRevenge = FALSE; /* ����ʹ�ܣ�����������ʧ��һ�������󣬸ñ���ΪTRUE��TRUE��ʾʹ�ܱ�����Ϊ���Ǳ����ڷ�����״̬������״̬�ɵ����߾�������
                                    FALSE��ʾ��ʹ�ܱ�����Ϊ */

bool8   G_ActivateMeterSuccess = FALSE; /* �����Ǳ�ɹ���־λ */

bool8   G_AllActivateCodeCheckFinish = FALSE; /* ���м���������ϣ�ƽʱ��鼤����ʱEEPROM�е������Ƿ�ʱ�ֶμ��ģ�
                                                 ÿ��һ����ʱ����һ�鼤����(ÿ�ν�����ѭ��ʱ���һ�μ�����)��
                                                 �����м�������һ����ٸ�λ����ʱ���� */
bool8   G_AllEEpromDataCheckSuccess = TRUE;  /* ����EEPROM���ݼ��ɹ���ƽʱ��鼤����ʱEEPROM�е������Ƿ�ʱ�ֶμ��ģ�
                                                ÿ��һ����ʱ����һ�鼤����(ÿ�ν�����ѭ��ʱ���һ�μ�����)��
                                                �����м�������һ����ٸ�λ����ʱ���� */
uint32_t TestTime_Start,TestTime_End;




/** bool8    ReadUniqueCode(uchar8  UniqueCodeArray[])     //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  ����UniqueCodeArray[]��Ϊ�����βΣ����ܵ�ͬ��ָ�룬��ʾ������Ԫ�صĵ�ַ��
*            ʵ�ʵ���ʱ�ñ���Ϊ����Ԫ��������������Ԫ�صĵ�ַ���Ҹ�����Ԫ�ظ���>=6��Ψһ���ȡ������������UniqueCodeArray[]�С�
*            
* ����ֵ˵��:  bool8, ��ȡΨһ��ɹ�����TRUE, ���򷵻�FALSE
*
* ���ܽ���:  ��ȡ����оƬ��6�ֽ�Ψһ��
*
* �ؼ���ע�� 1�����ڼ����Ǳ�ʱ�� �����ߵ��ô˺�����ȡΨһ�룬��Ψһ�뷢�͸�������
*               ��ȡ����Ψһ��ֱ�Ӵ����UniqueCodeArray[]��
*            2��24AA02E48T��Ψһ���ַ0xFA~0xFF
*               24AA02E48T memory: standard eeprom(00h~80h), write-protected node address block(80h~FFh)
*
* δ�����:
* 
*/ 
bool8	ReadUniqueCode(uchar8  UniqueCodeArray[])
{
	uchar8  i;                           
	uchar8  SuccessfulRead = FALSE;    /* ��ȡ�ɹ���0��ʾ��ȡʧ�ܣ�1��ʾ��ȡ�ɹ�	*/     
                                                                       
	SuccessfulRead = IICROM_LoadCharArrayData(UNIQUE_CODE_SAVE_ADDRESS, UniqueCodeArray, UNIQUE_CODE_BYTE_NUM); // Read 24AA02E48 ID(6 byte)
	if (SuccessfulRead == FALSE)
	{		  
		return FALSE;
	}
	  
	for (i = 0; i < UNIQUE_CODE_BYTE_NUM; i++)  /* Ψһ���ֽڸ���Ϊ6 */ 
	{
		if (UniqueCodeArray[i] != 0)           /* Ψһ�벻ȫΪ0 */ 
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



/** bool8   CheckCalculationCodeForActivation(uchar8  LocalUniqueCode[],uchar8  LocalCalcCode[])     //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  ������Ϊ�����βΣ����ܵ�ͬ��ָ�룬��ʾ������Ԫ�صĵ�ַ��ʵ�ʵ���ʱ�ñ���Ϊ����Ԫ��������������Ԫ�صĵ�ַ
*            LocalUniqueCode[]������оƬΨһ�룬Ψһ����6�ֽڣ�������Ԫ�ظ���>=6
*            LocalCalcCode[]������оƬ�ļ����룬��������8�ֽڣ�������Ԫ�ظ���>=8
*
* ����ֵ˵��:  bool8��FALSE��ʾ�����Ψһ����ͨ�������뷴�����Ψһ�벻һ�£�TRUE��ʾ������һ�µ�
*
* ���ܽ���:  ���Ϊ�����Ǳ����õļ�����
*
* �ؼ���ע��
*
* δ�����: 
* 
*/ 
bool8   CheckCalculationCodeForActivation(uchar8  LocalUniqueCode[], uchar8  LocalCalcCode[])
{
	uchar8  i;  
	uchar8  j;  	 
	uchar8  CalculatedUniqueCode[UNIQUE_CODE_BYTE_NUM] = {0}; /* �������Ψһ�� */   
	  
      
	  /**********************ͨ�������뷴���6�ֽ�Ψһ��**********************/      
	for (i = 0; i < UNIQUE_CODE_BYTE_NUM; i++)      /* ����оƬΨһ�룬Ψһ����6�ֽ� */ 
	{
		CalculatedUniqueCode[i] =  LocalCalcCode[i] - FIXED_VALUE;
		CalculatedUniqueCode[i] =  ~CalculatedUniqueCode[i]; 		       	
	}  
	  
	 /**********************ͨ��������������6�ֽ�Ψһ���������Ψһ��Ա�**********************/
	for (j = 0; j < UNIQUE_CODE_BYTE_NUM; j++)      /* ����оƬΨһ�룬Ψһ����6�ֽ� */ 
	{
		if (LocalUniqueCode[j] != CalculatedUniqueCode[j]) /* �����Ψһ����ͨ��������������Ψһ���Ƿ�һ�� */ 
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



/** bool8  CheckCompanyNameAndCopyRightDeclareInProgram(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, ���Ա�һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  �������еĹ�˾����
*
* �ؼ���ע��Ϊ�˷�ֹ ������Ա�ڱ�֤crc�����ǰ���½����޸ľֲ����ݣ����ǿ��԰����ݷֶ��ڽ���crcУ�飬
*           ���Բ�����İѰ�Ȩ�������зֶΣ�Ȼ��ÿ���ٽ���һ��crc������ֳ����Σ�ǰ25�ֽڣ���34�ֽڣ�
*
* δ�����:
* 
*/ 
bool8	CheckCompanyNameInProgram(void)
{
	uchar8  i;                           
	uint16_t TempParaCRC1,TempParaCRC2;
	
	/********************************�������еĹ�˾���ƽ���ÿ�ֽڵĶԱ�**************************/
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
	
	/***********************�������еĹ�˾���ƽ���CRCУ�飬�����μ���**************************/    	
	/* ��˾���Ƶ�һ��CRCУ�� */
	TempParaCRC1 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(G_CompanyName_RAM),COMPANY_NAME_CRC_BYTE1);
	if (TempParaCRC1 != COMPANY_NAME_CRC1)
	{
		return  FALSE;
	}
	else
	{
		;
	}
		
	/* ��˾���Ƶڶ���CRCУ�� */
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


/** bool8  CheckCopyRightDeclareInProgram(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, ���Ա�һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  �������еİ�Ȩ����
*
* �ؼ���ע��Ϊ�˷�ֹ ������Ա�ڱ�֤crc�����ǰ���½����޸ľֲ����ݣ����ǿ��԰����ݷֶ��ڽ���crcУ�飬
*           ���Բ�����İѰ�Ȩ�������зֶΣ�Ȼ��ÿ���ٽ���һ��crc. ����ֳ����Σ�ǰ50���м�100����167����
*
* δ�����:
* 
*/ 
bool8	CheckCopyRightDeclareInProgram(void)
{
	uint16  i;   
	uchar8  CopyRightDeclare_Ref[10] = {0};  /* ��Ȩ�����еĹؼ��ִ������ */
	uint16_t TempParaCRC1,TempParaCRC2,TempParaCRC3;	
	
	/***********************�������еİ�Ȩ�������йؼ��ʼ��**********************/
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
	 
	/***********************�������еİ�Ȩ��������CRCУ��**********************/	
	/* ��Ȩ������һ��CRCУ�� */
	TempParaCRC1 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(G_CopyRightDeclare),COPYRIGHT_DECLARE_CRC_BYTE1);
	if (TempParaCRC1 != COPYRIGHT_DECLARE_CRC1)
	{
		return  FALSE;
	}
	else
	{
		;
	}  
	
	/* ��Ȩ�����ڶ���CRCУ�� */
	TempParaCRC2 = HAL_CRC_Calculate(&hcrc,(uint32_t *)(&G_CopyRightDeclare[COPYRIGHT_DECLARE_CRC_BYTE1]),COPYRIGHT_DECLARE_CRC_BYTE2-COPYRIGHT_DECLARE_CRC_BYTE1);
	if (TempParaCRC2 != COPYRIGHT_DECLARE_CRC2)
	{
	    return  FALSE;
	}
	else
	{
		;
	}   
	
	/* ��Ȩ�����ڶ���CRCУ�� */
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



/** bool8  ReadCompanyNameFromEEProm( )   //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, �ɹ��Ļ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ��ȡ�洢��EEProm�еĹ�˾����
*
* �ؼ���ע��eeprom�Ķ�ȡ����ȽϺ�ʱ�����г���1mS�Ĺ������ܶ���Ҫ�ֳɶ������ɡ�
*
* δ�����: 
* 
*/ 
bool8	ReadCompanyNameFromEEProm(uint16_t COMPANYNAME_SAVE_ADDRESS,uint16_t CompanyNameReadByteNum)
{
	uchar8  i;  	
	bool8   ReadSuccess = TRUE;  /* ��ȡ�ɹ���־λ */			
//	bool8   SaveSuccess = FALSE; /* �洢�ɹ���־λ */		
	uchar8  ReadData[AT24C64_PAGE_BYTE_NUM];	     /* ��ȡ�������� */		
		
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

/** bool8  CheckPartialCompanyNameInEEProm(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ���洢��EEProm�еĹ�˾����
*
* �ؼ���ע��eeprom�Ķ�ȡ����ȽϺ�ʱ�����г���1mS�Ĺ������ܶ���Ҫ�ֳɶ������ɡ�
*
* δ�����: 
* 
*/ 
bool8	CheckPartialCompanyNameInEEProm(uint16_t COMPANYNAME_SAVE_ADDRESS,uint16_t *CompanyNameReadByteNum)
{	    
	bool8  ReadCompanyNameSuccess = FALSE;  /* ��ȡ�ɹ���־λ */	
	G_AllActivateCodeCheckFinish = FALSE;	    
	/***********************���EEProm�й�˾����**********************/
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

/** bool8  ReadAllCompanyNameFromEEProm(uint16 SaveAddress)   //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  SaveAddressΪ�洢��ַ��0 �� SaveAddress �� 8191
*
* ����ֵ˵��: bool8, �ɹ��Ļ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ��ȡ�洢��EEProm�еĵ�һ�ݹ�˾����ȫ��
*
* �ؼ���ע������������ѯ���Ǳ�����״̬ʱ�����洢��EEPROM�еĹ�˾����һ����ȫ������
*
* δ�����: 
* 
*/ 
bool8	ReadAllCompanyNameFromEEProm(uint16 SaveAddress)
{
	bool8  ReadCompanyNameSuccess = FALSE;  /* ��ȡ�ɹ���־λ */
	uint16 CompanyNameReadByteNum = 0;
	G_AllActivateCodeCheckFinish = FALSE;	    
	/***********************���EEProm�й�˾����**********************/
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



/** bool8  CheckAllCompanyNameInEEProm(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ���洢��EEProm�еĹ�˾����ȫ��
*
* �ؼ���ע������������ѯ���Ǳ�����״̬ʱ�����洢��EEPROM�еĹ�˾����һ����ȫ������
*
* δ�����: 
* 
*/ 
bool8	CheckAllCompanyNameInEEProm(void)
{	    
	bool8  ReadCompanyName1Success = FALSE;  /* ��ȡ�ɹ���־λ */
	bool8  ReadCompanyName2Success = FALSE;  /* ��ȡ�ɹ���־λ */
	
	/***********************���EEProm�е�һ�ݹ�˾����**********************/
	ReadCompanyName1Success = ReadAllCompanyNameFromEEProm(COMPANYNAME_SAVE_ADDRESS1);   
	/***********************���EEProm�еڶ��ݹ�˾����**********************/
		   
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


/** bool8  ReadCopyRightDeclareFromEEProm( )   //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, �ɹ��Ļ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ��ȡ�洢��EEProm�еİ�Ȩ����
*
* �ؼ���ע��eeprom�Ķ�ȡ����ȽϺ�ʱ�����г���1mS�Ĺ������ܶ���Ҫ�ֳɶ������ɡ�
*
* δ�����: 
* 
*/ 
bool8	ReadCopyRightDeclareFromEEProm()
{
	uchar8  i;  	
	bool8   ReadSuccess = TRUE;  /* ��ȡ�ɹ���־λ */			
//	bool8   SaveSuccess = FALSE; /* �洢�ɹ���־λ */		
	uchar8  ReadData[AT24C64_PAGE_BYTE_NUM];	     /* ��ȡ�������� */		
		
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


/** bool8  CheckPartialCopyRightDeclareInEEProm(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  �������EEProm�еĲ��ְ�Ȩ����
*
* �ؼ���ע��eeprom�Ķ�ȡ����ȽϺ�ʱ�����г���1mS�Ĺ������ܶ���Ҫ�ֳɶ������ɡ�
*
* δ�����: 
* 
*/ 
bool8	CheckPartialCopyRightDeclareInEEProm(void)
{
	bool8  ReadCopyRightSuccess = FALSE;  /* ��ȡ�ɹ���־λ */
	
       /***********************���EEProm�а�Ȩ����**********************/	    	   
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



/** bool8  ReadAllCopyRightDeclareFromEEProm( )   //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, �ɹ��Ļ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ��ȡ�洢��EEProm�еİ�Ȩ����ȫ��
*
* �ؼ���ע������������ѯ���Ǳ�����״̬ʱ�����洢��EEPROM�еİ�Ȩ����һ����ȫ������
*
* δ�����: 
* 
*/ 
bool8	ReadAllCopyRightDeclareFromEEProm()
{
	uint16  i;
	uchar8 TempC;
	bool8   ReadSuccess = TRUE;  /* ��ȡ�ɹ���־λ */	
//	bool8   SaveSuccess = FALSE;  /* �洢�ɹ���־λ	*/	
	uchar8  ReadData[AT24C64_PAGE_BYTE_NUM];	     /* ��ȡ�������� */		
		
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




/** bool8  CheckAllCopyRightDeclareInEEProm(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  �������EEProm�еİ�Ȩ����ȫ��
*
* �ؼ���ע������������ѯ���Ǳ�����״̬ʱ�����洢��EEPROM�еİ�Ȩ����һ����ȫ������
*
* δ�����: 
* 
*/ 
bool8	CheckAllCopyRightDeclareInEEProm(void)
{
	bool8  ReadCopyRightSuccess = FALSE;  /* ��ȡ�ɹ���־λ */
	   
	   	
       /***********************���EEProm�а�Ȩ����**********************/
	    	   			    
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



/** bool8  CheckCompanyNameAndCopyRightDeclare(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, ���Ա�һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ��鹫˾���ƺͰ�Ȩ����
*            ��˾���ƺͰ�Ȩ�����ļ���Ϊ����: ͨ��G_EnableActivatorAskState������
*            1�������ļ��: ��ʱ�ֶμ�飬��ÿ��һ����ʱ����м�����ļ�飬���ڼ�����϶�
*               ��ƽʱһ����ȫ�����һ��Ļ������ɽϴ����ÿִ��һ�θú����ͼ�����е�һ��
*               �����룬ֱ��������ȫ�����һ�飬�ٽ���ʱ�������㣬���ѭ����
*            2����ͨ��ʹG_EnableActivatorAskState = 1��ʹ�����м�����һ��ȫ�������һ��
*               ������ͨ��Һ���˻�ȡ�Ǳ�ļ�������ȷ��񣬻��߳�ʼ�ϵ�ʱ�����м�����ȫ�����һ��
*               ���Ǳ�ļ�������ȷ��񣬲���������ʶ��ʾ��Һ����
*
* �ؼ���ע��
*
* δ�����:
* 
*/ 
bool8	CheckCompanyNameAndCopyRightDeclare(void)
{ 
	bool8  CompanyNameCorrect1 = FALSE;
	bool8  CompanyNameCorrect2 = FALSE;
	bool8  CompanyNameCorrect3 = FALSE;
	bool8  CopyRightCorrect1 = FALSE;
	bool8  CopyRightCorrect2 = FALSE;
	   
	if (G_EnableActivatorAskState == FALSE)  /* ������鼤����: ��ʱ�ֶμ��*/
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
	else  /* ��ʱ��ѯ�����룬һ���Խ����м�������һ�� */
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


/** bool8  CheckValidityOfCalculationCode(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, ������Ϸ�����TRUE�����򷵻�FALSE
*
* ���ܽ���:  �������뼤��Ϸ���
*
* �ؼ���ע�����øú�����ǰ�����Ѿ��Ӽ���оƬ�ж�ȡ��������
*
* δ�����:
* 
*/ 
bool8	CheckValidityOfCalculationCode(void)
{
	bool8  ReadSuccess = FALSE;  /* �ɹ���ȡΨһ�룬TRUE����ɹ��� FALSE����ʧ�� */
	bool8  CalcCodeCompareResult = FALSE;  /* ������ȽϽ����TRUE����ɹ��� FALSE����ʧ�� */	  
	  
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



/** bool8  CheckCalculationCodeAndReverseCodeAndFixedCode(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, ��ȡ���ļ����루ԭ�롢�䷴���Լ�ԭ��+�̶��룩>=2��һ�µĻ�������TRUE�����򷵻�FALSE
*
* ���ܽ���:  ��������ԭ�롢�䷴���Լ�ԭ��+�̶���
*
* �ؼ���ע��3�ݲ�ͬ�汾�����3�ݶ��ǶԵģ���Ϊ��������ȷ��
*                        ���2�ݶԵģ�һ�ݴ�����Ϊ��������ȷ����������������ݡ�
*                        ���2�ݴ������3�ݶ���������Ϊʧ�ܣ����޸���������������Ϊ��
*
* δ�����:
* 
*/ 
bool8	CheckCalculationCodeAndReverseCodeAndFixedCode(void)
{
	uchar8  i;  
	uchar8  j;  
	uchar8  k;  
	  
	bool8   ReadSourceCodeSuccess = FALSE;   /* ��ȡԭ��ɹ���־λ */
	bool8   ReadReverseCodeSuccess = FALSE;  /* ��ȡ����ɹ���־λ */
	bool8   ReadFixedCodeSuccess = FALSE;    /* ��ȡ�̶���ɹ���־λ */
	  
	bool8   SourceReverseCodeCheckResult = FALSE;  /* ԭ�롢����ȽϽ�� */
	bool8   SourceFixedCodeCheckResult = FALSE;    /* ԭ�롢�̶���ȽϽ�� */
	bool8   ReverseFixedCodeCheckResult = FALSE;   /* ���롢�̶���ȽϽ�� */
	  
	uchar8  CalcCodeErrorCount = 0;          /* ������������ */
	  
	uchar8  ReverseCode_Source[CALC_CODE_BYTE_NUM] = {0};   /* ��ԭ�������ķ��� */
	uchar8  FixedCode_Source[FIXED_CODE_BYTE_NUM] = {0};    /* ��ԭ�������Ĺ̶��� */
	uchar8  ReverseCode_Fixed[CALC_CODE_BYTE_NUM] = {0};    /* �ɹ̶��������ķ��� */
      
	ReadSourceCodeSuccess = IICROM_LoadCharArrayData(SOURCE_CODE_SAVE_ADDRESS, G_CalcSourceCode, CALC_CODE_BYTE_NUM);
	if (ReadSourceCodeSuccess == FALSE)
	{
		return FALSE;     
	} 
			  
	ReadReverseCodeSuccess = IICROM_LoadCharArrayData(REVERSE_CODE_SAVE_ADDRESS,G_CalcReverseCode, CALC_CODE_BYTE_NUM); //�����뷴��Ƚ�
	if (ReadReverseCodeSuccess == FALSE)
	{
		return FALSE;
	}
		  
	ReadFixedCodeSuccess = IICROM_LoadCharArrayData(FIXED_CODE_SAVE_ADDRESS, G_CalcFixedCode, FIXED_CODE_BYTE_NUM); //������ԭ��+�̶���Ƚ�
	if (ReadFixedCodeSuccess == FALSE)
	{
		return FALSE;	
	}
            
	for (i = 0; i < CALC_CODE_BYTE_NUM; i++)  /* ԭ�롢����Ƚ� */
	{
		ReverseCode_Source[i] = ~G_CalcSourceCode[i];
		if (G_CalcReverseCode[i] != ReverseCode_Source[i])
		{
			SourceReverseCodeCheckResult = FALSE;
			CalcCodeErrorCount++;
			break;
		}	
	}      
	  
	for (j = 0; j < FIXED_CODE_BYTE_NUM; j++)  /* ԭ�롢�̶���Ƚ� */
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
	  
	for (k = 0; k < CALC_CODE_BYTE_NUM; k++)  /* ���롢�̶���Ƚ� */
	{
		ReverseCode_Fixed[k] = ~G_CalcFixedCode[k];
		if (G_CalcReverseCode[k] != ReverseCode_Fixed[k])
		{
			ReverseFixedCodeCheckResult = FALSE;
			CalcCodeErrorCount++;
			break;
		}	
	}      
	    
	/***********************�Լ�������з�������*********************/
	if (CalcCodeErrorCount == 0)      /* 3�ݶ��ǶԵģ���Ϊ��������ȷ */
	{
		return TRUE;
	}
	else if (CalcCodeErrorCount == 2)   /* ����Ա���ֻ��һ���ǶԵģ���3������2���ǶԵģ���Ϊ��������ȷ����������������ݡ�*/
	{
		if ( (SourceReverseCodeCheckResult  == FALSE) && (SourceFixedCodeCheckResult == FALSE) )  /* ԭ���뷴�롢ԭ����̶���ȽϾ�����*/
		{				  
			IICROM_SaveCharArrayData(SOURCE_CODE_SAVE_ADDRESS, G_CalcFixedCode, CALC_CODE_BYTE_NUM);  /* save calculation source code( 8 byte )  */                
		}
		else if ( (SourceReverseCodeCheckResult  == FALSE) && (ReverseFixedCodeCheckResult == FALSE) )  /* ԭ���뷴�롢������̶���ȽϾ�����*/ 
		{
			IICROM_SaveCharArrayData(REVERSE_CODE_SAVE_ADDRESS, ReverseCode_Source, CALC_CODE_BYTE_NUM);  /* save calculation reverse code( 8 byte ) */                 
              
		}
		else if ( (SourceFixedCodeCheckResult  == FALSE) && (ReverseFixedCodeCheckResult == FALSE) )  /* ԭ����̶��롢������̶���ȽϾ�����*/  
		{
			IICROM_SaveCharArrayData(FIXED_CODE_SAVE_ADDRESS, FixedCode_Source, FIXED_CODE_BYTE_NUM); /* save calculation fixed code( 10 byte ) */                   
             
		}		
		return TRUE;
	}	
	else 		/* 3���������2�ݴ������3�ݶ���������Ϊʧ�ܣ����޸� */
	{
		return FALSE;
	}     
}


/** bool8   CheckCorrectnessOfEncryptionChip(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8�� TRUE�������оƬ��ȷ��FALSE�������оƬ����ȷ
*
* ���ܽ���:  ������оƬ����ȷ��
*
* �ؼ���ע������оƬ��80h---FFhΪд�����������ô洢������һ��ַд�����ݣ��ٶ�ȡ������д��ĺͶ�ȡ���ǲ�һ����
*
* δ�����: 
* 
*/ 
bool8	CheckCorrectnessOfEncryptionChip(void)
{
	uchar8  WriteData[AA02E48_PAGE_BYTE_NUM] = {0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80};  /* ��д���� */
	bool8   SaveSuccess = FALSE;  /* �洢�ɹ���־λ */
	SaveSuccess = IICROM_SaveCharArrayData(WRITE_PROTECT_SAVE_ADDRESS,WriteData,AA02E48_PAGE_BYTE_NUM);
	if (SaveSuccess == TRUE)  /* ����оƬд�������Ǵ洢���˵ģ�����洢�ɹ���֤�����Ǽ���оƬ */
	{
		RecordPerfToFRAM(WRITEENCRYPTIONCHIPERRORCOUNT);
		return FALSE;
	}
	else
	{
		return TRUE;	   
	}
}


/** bool8   CheckCalculationCode(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8�� TRUE�����������ȷ��FALSE��������벻��ȷ
*
* ���ܽ���:  ��������, ��������оƬ����ȷ�ԡ����������ȷ���Լ������뼤��Ϸ��ԡ�
*
* �ؼ���ע��
*
* δ�����: 
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



/** bool8   CheckActivationCode(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8�� TRUE����������ȷ��FALSE�������벻��ȷ
*             ����ֵ��ʹ��: �ڳ�ʼ�ϵ�����м�������һ��ʱ��ʹ�ø�bool8�ͷ���ֵ
*             ��������while(1)���õ����Ǽ�������ʧ�ܴ���G_ActivateCodeErrorCount������G_ActivateCodeErrorCount
*             �Ĵ�С�ж��Ƿ���뱨����Ϊ״̬
*
* ���ܽ���:  ��鼤����, ԼCHECK_ACTIVATIONCODE_TIME(1800,��Сʱ)���һ�Σ��ۼƼ��ʧ�ܵĴ�����
*            ʧ�ܴ������ֵ����CHECK_ACTIVATIONCODE_FAIL_NUM(10��)
*            ���嶨ʱʱ��CHECK_ACTIVATIONCODE_TIME�Լ�ʧ�ܴ�������CHECK_ACTIVATIONCODE_FAIL_NUM���ɵ������Լ��趨
*
* �ؼ���ע��������ļ�����G_EnableActivatorAskState��ֵ��Ϊ���ֹ���״̬: 
*           (1)���ʱ���: �͸�������(��������Ĺ�����ƽ�����䵽ÿһ�γ��������)������ѭ����ִ��  
*           (2)ֻ��ѯ�ʼ���״̬ʱ���: ȫ�����һ�飬Ŀǰֻ���ڳ�ʼ�ϵ�ʱ�Լ���������ѯ�ʼ���״̬ʱ��ִ�����ֹ���״̬
*
* δ�����: 
* 
*/ 
bool8	CheckActivationCode(void)
{   
	bool8  CalcCodeCorrect = FALSE;          /* ����������ȷ����־λ */
	bool8  NameAndCopyRightCorrect = FALSE;  /* ����˾���ƺͰ�Ȩ������ȷ����־λ */
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


/** bool8  RevengeOrNot(void)    //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ:  
*
* ����ֵ˵��: bool8, FALSE��ʾ��ʵʩ������Ϊ��TRUE��ʾʵʩ������Ϊ
*
* ���ܽ���:  �Ƿ�ʵʩ������Ϊ����������Ϊ������������ģʽ��������ɱ��������Լ�������
*
* �ؼ���ע��
*
* δ�����:
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



/** bool8  SaveCalcCodeToEncryptionChip(void)     //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ: 
*
* ����ֵ˵��:  bool8������ɹ�����TRUE�����򷵻�FALSE 
*
* ���ܽ���: �������루ԭ�롢�����Լ��̶���(ԭ��+0xA5+0x5A)���洢�ڼ���оƬ24AA02E48T��Standard EEPROM��           
*
* �ؼ���ע��24AA02E48T��0x00~0x80Ϊ��׼EEPROM����0x80~0xFFΪ24AAE0248TΨһ��洢����д������
*           Page Write Time 3 ms, typical
*           Page Write Buffer:  8-byte page (24AA02E48/24AA02E64)  
*
* δ�����:
* 
*/
bool8	SaveCalcCodeToEncryptionChip(void)
{
	uchar8  i;            
	uchar8  j;             
	bool8   CalcSourceCodeSuccessfulSave = FALSE;    /* ������ԭ��ɹ����棬TRUE��ʾ����ɹ���FALSE ��ʾ����ʧ�� */
	bool8   CalcReverseCodeSuccessfulSave = FALSE;	 /* �����뷴��ɹ����棬TRUE��ʾ����ɹ���FALSE ��ʾ����ʧ�� */
	bool8   CalcFixedCodeSuccessfulSave = FALSE;     /* ������̶���ɹ����棬TRUE��ʾ����ɹ���FALSE ��ʾ����ʧ�� */
	
	/* ���������ԭ��,8�ֽ� */
	CalcSourceCodeSuccessfulSave = IICROM_SaveCharArrayData(SOURCE_CODE_SAVE_ADDRESS, G_CalcSourceCode, CALC_CODE_BYTE_NUM); 
	if (CalcSourceCodeSuccessfulSave == FALSE)
	{
	return FALSE;
	}
	
	/*  ��������뷴��,8�ֽ� */		
	for (i = 0; i < CALC_CODE_BYTE_NUM; i++)                           
	{
	     G_CalcReverseCode[i] = ~G_CalcSourceCode[i];
	}		 
	CalcReverseCodeSuccessfulSave = IICROM_SaveCharArrayData(REVERSE_CODE_SAVE_ADDRESS, G_CalcReverseCode, CALC_CODE_BYTE_NUM);  
	if (CalcReverseCodeSuccessfulSave == FALSE)
	{
        return FALSE; 
	}
	
	/*  ���������̶���,10�ֽ� */
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


/** uchar8  SaveCopyRightDeclare(void)     //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ: 
*
* ����ֵ˵��: uchar8 ������ɹ�����1�����򷵻�0
*
* ���ܽ���:  �����Ȩ���� 
*
* �ؼ���ע�� 
*
* δ�����:
* 
*/
bool8	SaveCopyRightDeclare(void)
{	    
	bool8   SaveSuccess = FALSE;  /* ����ɹ���־λ */
		
	/************************�洢��Ȩ����************************/
	SaveSuccess = ReadAllCopyRightDeclareFromEEProm();
//	if(SaveSuccess == FALSE)
//	{
//		WriteEeprom(COPYRIGHT_SAVE_ADDRESS, (uint8_t *)(G_CopyRightDeclare),sizeof(G_CopyRightDeclare));
//		IICRomDelay(IICROM_DELAY_5MS);
//		SaveSuccess = ReadAllCopyRightDeclareFromEEProm();
//	}
	return (SaveSuccess);	
}



/** bool8  SaveCompanyNameToEEProm(uint16 SaveAddress)     // �������ƺͲ����ͷ���ֵ���͡�
* ������Χ: SaveAddressΪ�洢��ַ��0 �� SaveAddress �� 8191
*
* ����ֵ˵��: bool8������ɹ�����TRUE�����򷵻�FALSE
*
* ���ܽ���:  ���湫˾���Ƶ�EEPROM��
*
* �ؼ���ע�� 1��Ϊ��֤�洢�Ŀɿ��ԣ���˾���Ʊ������ݣ�
*               ��Ϊ����оƬ24AA02E48T�Ŀ�д�洢������Ϊ00H~7FH����128�ֽڣ��洢�ռ䲻���洢��˾���ƣ���˽���洢��AT24C64��
*            2��AT24CXXд����ʱ��Ѱַ�豸����Ƭ��������ֹͣ��������ֹд���У���ʱ��EEPROM���뵽�ڲ��Լ�ʱд���ڣ����5ms��
*               ��д�����ڼ����е����벻ʹ�ܣ���EEPROM��д������ɺ�Ż���Ӧ
*
* δ�����:
* 
*/
bool8  SaveCompanyNameToEEProm(uint16 SaveAddress)
{	
	bool8   SaveCompanyNameSuccess = FALSE;          /* �ɹ����湫˾���ƣ�TRUE����ɹ��� FALSE����ʧ�� */		
	
	/* ��ҳ�洢���� */
        if (SaveCompanyNameSuccess == FALSE)
	{
		return FALSE;	
	}
	return TRUE;
}


/** bool8  SaveCompanyName(void)     // �������ƺͲ����ͷ���ֵ���͡�
* ������Χ: 
*
* ����ֵ˵��: bool8������ɹ�����TRUE�����򷵻�FALSE
*
* ���ܽ���:  ���湫˾���� 
*
* �ؼ���ע�� 1��Ϊ��֤�洢�Ŀɿ��ԣ���˾���Ʊ������ݣ�
*               ��Ϊ����оƬ24AA02E48T�Ŀ�д�洢������Ϊ00H~7FH����128�ֽڣ��洢�ռ䲻���洢��˾���ƣ���˽���洢��AT24C64��
*            2��AT24CXXд����ʱ��Ѱַ�豸����Ƭ��������ֹͣ��������ֹд���У���ʱ��EEPROM���뵽�ڲ��Լ�ʱд���ڣ����5ms��
*               ��д�����ڼ����е����벻ʹ�ܣ���EEPROM��д������ɺ�Ż���Ӧ
*
* δ�����:
* 
*/
bool8	SaveCompanyName(void)
{	   	
	bool8   SaveCompanyName1Success = FALSE;           /* �ɹ������һ�ݹ�˾���ƣ�TRUE����ɹ��� FALSE����ʧ�� */
	bool8   SaveCompanyName2Success = FALSE;           /* �ɹ�����ڶ��ݹ�˾���ƣ�TRUE����ɹ��� FALSE����ʧ�� */ 
	   
		
	/***********************�洢��һ�ݹ�˾����*********************/
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
		
	/**********************�洢�ڶ��ݹ�˾����**********************/
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


/** bool8  SaveCalculationCode(void)     //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ: 
*
* ����ֵ˵��: bool8������ɹ�����TRUE�����򷵻�FALSE
*
* ���ܽ���:  ���������  
*
* �ؼ���ע�� ���ǵ��洢�Ŀɿ��ԣ����ǰѼ�����洢3�ݲ�ͬ�İ汾��
*            �汾1��������ԭ��-------------------------------------8�ֽ�
*            �汾2�������뷴��-------------------------------------8�ֽ�
*            �汾3��������ԭ��+�̶��루0xA5,0x5A��-----------------10�ֽ�
*
* δ�����:
* 
*/
bool8	SaveCalculationCode(void)
{
	bool8  ReadSuccess = FALSE;  /* �ɹ���ȡΨһ�룬TRUE����ɹ��� FALSE����ʧ�� */
	bool8  SaveSuccess = FALSE;  /* �ɹ���������룬TRUE����ɹ��� FALSE����ʧ�� */
	bool8  CalcCodeCompareResult = FALSE;  /* ������ȽϽ����TRUE����ɹ��� FALSE����ʧ��  */
	  
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


/** bool8  ActivateMeter(void)     //�������ƺͲ����ͷ���ֵ���͡�
* ������Χ: 
*
* ����ֵ˵��: bool8 ���Ǳ���ɹ�����TRUE�����򷵻�FALSE
*
* ���ܽ���:  �����Ǳ�
*            �����Ǳ�Ĺ���ʵ�ʾ������ɲ��洢������Ĺ��̣������������Ψһ�������ļ�����ԭ�롢���롢ԭ��+�̶��롢
*            ��˾���ơ���Ȩ��������Щ������洢�ɹ��ʹ����Ǳ���ɹ�������ʹ����Ǳ���ʧ��
*
* �ؼ���ע�� �ú�������ʹ�ü������������Ǳ�ʱ�����Ǳ��յ��������������ļ�����ʱ�ٵ��øú���
*
* δ�����:
* 
*/
bool8	ActivateMeter(void)
{
	bool8  CalcCodeSuccessfulSave = FALSE;      /* ������ɹ����棬FALSE��ʾ���ɹ���TRUE��ʾ�ɹ� */
	bool8  CompanyNameSuccessfulSave = FALSE;   /* ��˾���Ƴɹ����棬FALSE��ʾ���ɹ���TRUE��ʾ�ɹ� */
	bool8  CopyRightSuccessfulSave = FALSE;     /* ��Ȩ�����ɹ����棬FALSE��ʾ���ɹ���TRUE��ʾ�ɹ� */
	  
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
