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



#ifndef __COPYRIGHTACTIVATE_H
#define __COPYRIGHTACTIVATE_H


#include "DataType.h"





/****************************CopyRightActivate.h�ļ�˵��********************************************/
// ���ļ�ΪCopyRightActivate.c����һЩ���⿪�ŵĽӿ�
/***************************************************************************************************/

#define  CHECK_ACTIVATIONCODE_FAIL_NUM		10                        /* ��ʱ�������롢��˾���ơ���Ȩ����ʧ�ܴ���CHECK_ACTIVATIONCODE_FAIL_NUM�Σ��ﵽ�˴������б�����Ϊ */
#define  CHECK_ACTIVATIONCODE_TIME		1800                      /* ��λ���룬��ʱʱ��������ʱ�������롢��˾���ơ���Ȩ��������ʱ���ʵ��ֵΪ�ú궨����ֵ�붨ʱ����ʱ����ĳ˻���*/

#define  UNIQUE_CODE_BYTE_NUM			6                         /* Ψһ���ֽ��� */
#define  CALC_CODE_BYTE_NUM			8                         /* �������ֽ��� */

#define  AA02E48_PAGE_BYTE_NUM       	    8         /* 24AA02E48֧��ҳдPage Write Buffer:8-byte page (24AA02E48/24AA02E64) */

/* 24AA02E48T memory: standard eeprom(00h~80h), write-protected node address block(80h~FFh) */
#define  SOURCE_CODE_SAVE_ADDRESS           0x10      /* ������ԭ��洢��ַ(�洢�ڼ���оƬ24AA02E48) */
#define  REVERSE_CODE_SAVE_ADDRESS          0x18      /* �����뷴��洢��ַ(�洢�ڼ���оƬ24AA02E48) */
#define  FIXED_CODE_SAVE_ADDRESS            0x20      /* ������̶���洢��ַ(�洢�ڼ���оƬ24AA02E48) */
#define  WRITE_PROTECT_SAVE_ADDRESS         0xA0      /* д�����洢��ַ(�洢�ڼ���оƬ24AA02E48) */

#define  AT24C64_PAGE_BYTE_NUM              32        /* AT24C64֧��ҳд��ÿҳ32�ֽڣ�,Self-timed Write Cycle (5ms Max) */

#define  COMPANYNAME_SAVE_ADDRESS1          6400      /* (�洢��AT24C64) 6080*/
#define  COMPANYNAME_SAVE_ADDRESS2          6496      /* (�洢��AT24C64) 6176*/
#define  COPYRIGHT_SAVE_ADDRESS             6688      /* (�洢��AT24C64) 6368*/



#define  COMPANY_NAME_CRC_BYTE1             25       /* ��˾�������ڼ����һ��CRC���ֽ��� */
#define  COMPANY_NAME_CRC1                  0x9C94   /* ��˾����ǰ25���ֽڵ�CRC���ú궨���ֵ��ͨ���������������Ƭ��Ӳ��CRC��������� */
#define  COMPANY_NAME_CRC2                  0x4969   /* ��˾���ƺ�34���ֽڵ�CRC���ú궨���ֵ��ͨ���������������Ƭ��Ӳ��CRC��������� */


#define  COPYRIGHT_DECLARE_CRC_BYTE1        50       /* ��Ȩ�������ڼ����һ��CRC���ֽ��� */
#define  COPYRIGHT_DECLARE_CRC_BYTE2        150      /* ��Ȩ�������ڼ���ڶ���CRC���ֽ��� */

#define  COPYRIGHT_DECLARE_CRC1             0xA690   /* ��Ȩ����ǰ50���ֽڵ�CRC���ú궨���ֵ��ͨ���������������Ƭ��Ӳ��CRC��������� */
#define  COPYRIGHT_DECLARE_CRC2             0x3A38   /* ��Ȩ�����м�100���ֽڵ�CRC���ú궨���ֵ��ͨ���������������Ƭ��Ӳ��CRC��������� */
#define  COPYRIGHT_DECLARE_CRC3             0x7A29   /* ��Ȩ������167���ֽڵ�CRC���ú궨���ֵ��ͨ���������������Ƭ��Ӳ��CRC��������� */

#define  COPYRIGHT_DECLARE_KEYWORD_START    67       /* ��Ȩ�����ؼ��ּ�����ʼλ�� */
#define  COPYRIGHT_DECLARE_KEYWORD_END      77       /* ��Ȩ�����ؼ��ּ��Ľ���λ�� */

extern uint16  G_CheckActivateCodeTimer;         /* ���ڼ�鼤����(�����롢��˾���ơ���Ȩ����)�õļ�ʱ����,���ø��ļ�ʱ���ñ������ڶ�ʱ���ж��н���++
                                                  �� >= CHECK_ACTIVATIONCODE_TIME ʱ����һ�μ������� */

extern uchar8  G_UniqueCode[UNIQUE_CODE_BYTE_NUM];         /* ����оƬΨһ�������� */
extern uchar8  G_CalcSourceCode[CALC_CODE_BYTE_NUM];       /* ����оƬ������ԭ�������� */
extern bool8   G_EnableActivatorAskState;  /* ������ѯ�ʼ���״̬ʹ�ܣ���鼤����ʱ�����֣�һ����ϵͳ��ʱ��飬��������Ĺ�����ƽ�����䵽ÿһ�γ��������
                                              ����һ���ǵ�������ѯ���Ǳ�ļ���״̬ʱ�������м�����Ϣ���һ�� */

extern bool8   G_EnableRevenge;     /* ����ʹ�ܣ�����������ʧ��һ�������󣬸ñ���ΪTRUE��TRUE��ʾʹ�ܱ�����Ϊ���Ǳ����ڷ�����״̬������״̬�ɵ����߾�����,
                                       ALSE��ʾ��ʹ�ܱ�����Ϊ */
extern bool8   G_ActivateMeterSuccess; /* �����Ǳ�ɹ���־λ */
                                            
//extern uchar8  G_ActivateCodeErrorCount;  /* ������(�����롢��˾���ơ���Ȩ����)�������,��鵽���������ʱ�ñ���+1
//                                             �� >= CHECK_ACTIVATIONCODE_FAIL_NUMʱ���ñ�����Ϊ */

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
bool8   ReadUniqueCode(uchar8 UniqueCodeArray[]);

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
bool8   ActivateMeter(void);

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
bool8   CheckActivationCode(void);

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
bool8   RevengeOrNot(void);
uchar8	Read_ActivateCodeErrorCount(void);
void	IncCheckActivateCodeTimer(void);

#endif
