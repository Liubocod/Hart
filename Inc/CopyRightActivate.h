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



#ifndef __COPYRIGHTACTIVATE_H
#define __COPYRIGHTACTIVATE_H


#include "DataType.h"





/****************************CopyRightActivate.h文件说明********************************************/
// 该文件为CopyRightActivate.c声明一些对外开放的接口
/***************************************************************************************************/

#define  CHECK_ACTIVATIONCODE_FAIL_NUM		10                        /* 定时检查计算码、公司名称、版权声明失败次数CHECK_ACTIVATIONCODE_FAIL_NUM次，达到此次数进行报复行为 */
#define  CHECK_ACTIVATIONCODE_TIME		1800                      /* 单位：秒，定时时间间隔，定时检查计算码、公司名称、版权声明（定时间隔实际值为该宏定义数值与定时器定时间隔的乘积）*/

#define  UNIQUE_CODE_BYTE_NUM			6                         /* 唯一码字节数 */
#define  CALC_CODE_BYTE_NUM			8                         /* 计算码字节数 */

#define  AA02E48_PAGE_BYTE_NUM       	    8         /* 24AA02E48支持页写Page Write Buffer:8-byte page (24AA02E48/24AA02E64) */

/* 24AA02E48T memory: standard eeprom(00h~80h), write-protected node address block(80h~FFh) */
#define  SOURCE_CODE_SAVE_ADDRESS           0x10      /* 计算码原码存储地址(存储在加密芯片24AA02E48) */
#define  REVERSE_CODE_SAVE_ADDRESS          0x18      /* 计算码反码存储地址(存储在加密芯片24AA02E48) */
#define  FIXED_CODE_SAVE_ADDRESS            0x20      /* 计算码固定码存储地址(存储在加密芯片24AA02E48) */
#define  WRITE_PROTECT_SAVE_ADDRESS         0xA0      /* 写保护存储地址(存储在加密芯片24AA02E48) */

#define  AT24C64_PAGE_BYTE_NUM              32        /* AT24C64支持页写（每页32字节）,Self-timed Write Cycle (5ms Max) */

#define  COMPANYNAME_SAVE_ADDRESS1          6400      /* (存储在AT24C64) 6080*/
#define  COMPANYNAME_SAVE_ADDRESS2          6496      /* (存储在AT24C64) 6176*/
#define  COPYRIGHT_SAVE_ADDRESS             6688      /* (存储在AT24C64) 6368*/



#define  COMPANY_NAME_CRC_BYTE1             25       /* 公司名称用于计算第一段CRC的字节数 */
#define  COMPANY_NAME_CRC1                  0x9C94   /* 公司名称前25个字节的CRC，该宏定义的值是通过仿真根据瑞萨单片机硬件CRC计算出来的 */
#define  COMPANY_NAME_CRC2                  0x4969   /* 公司名称后34个字节的CRC，该宏定义的值是通过仿真根据瑞萨单片机硬件CRC计算出来的 */


#define  COPYRIGHT_DECLARE_CRC_BYTE1        50       /* 版权声明用于计算第一段CRC的字节数 */
#define  COPYRIGHT_DECLARE_CRC_BYTE2        150      /* 版权声明用于计算第二段CRC的字节数 */

#define  COPYRIGHT_DECLARE_CRC1             0xA690   /* 版权声明前50个字节的CRC，该宏定义的值是通过仿真根据瑞萨单片机硬件CRC计算出来的 */
#define  COPYRIGHT_DECLARE_CRC2             0x3A38   /* 版权声明中间100个字节的CRC，该宏定义的值是通过仿真根据瑞萨单片机硬件CRC计算出来的 */
#define  COPYRIGHT_DECLARE_CRC3             0x7A29   /* 版权声明后167个字节的CRC，该宏定义的值是通过仿真根据瑞萨单片机硬件CRC计算出来的 */

#define  COPYRIGHT_DECLARE_KEYWORD_START    67       /* 版权声明关键字检查的起始位置 */
#define  COPYRIGHT_DECLARE_KEYWORD_END      77       /* 版权声明关键字检查的结束位置 */

extern uint16  G_CheckActivateCodeTimer;         /* 定期检查激活码(计算码、公司名称、版权声明)用的计时变量,调用该文件时将该变量放在定时器中断中进行++
                                                  当 >= CHECK_ACTIVATIONCODE_TIME 时进行一次激活码检查 */

extern uchar8  G_UniqueCode[UNIQUE_CODE_BYTE_NUM];         /* 加密芯片唯一码存放数组 */
extern uchar8  G_CalcSourceCode[CALC_CODE_BYTE_NUM];       /* 加密芯片计算码原码存放数组 */
extern bool8   G_EnableActivatorAskState;  /* 激活器询问激活状态使能，检查激活码时分两种，一种是系统定时检查，把整体检查的工作量平均分配到每一次程序调用中
                                              另外一种是当激活器询问仪表的激活状态时，把所有激活信息检查一下 */

extern bool8   G_EnableRevenge;     /* 报复使能，当激活码检查失败一定次数后，该变量为TRUE，TRUE表示使能报复行为（仪表工作在非正常状态，具体状态由调用者决定）,
                                       ALSE表示不使能报复行为 */
extern bool8   G_ActivateMeterSuccess; /* 激活仪表成功标志位 */
                                            
//extern uchar8  G_ActivateCodeErrorCount;  /* 激活码(计算码、公司名称、版权声明)错误次数,检查到激活码错误时该变量+1
//                                             当 >= CHECK_ACTIVATIONCODE_FAIL_NUM时启用报复行为 */

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
bool8   ReadUniqueCode(uchar8 UniqueCodeArray[]);

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
bool8   ActivateMeter(void);

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
bool8   CheckActivationCode(void);

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
bool8   RevengeOrNot(void);
uchar8	Read_ActivateCodeErrorCount(void);
void	IncCheckActivateCodeTimer(void);

#endif
