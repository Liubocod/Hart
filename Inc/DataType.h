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




/***********************DataType.h�ļ�˵��**************************************
���ļ���Ҫ�Ǹ��ݲ�ͬ��mcu�������ͺ��������õ������������Զ�Ӧ��

��Ҫ�õ�����������uchar8��uint16��char8��int16��float32

�����ļ��е��������;��������������֡�
Ȼ���ڲ�ͬ��mcuƽ̨ͨ�����ļ���������mcu�ṩ���������Ͷ�Ӧ��
��Ҫ��һ���ĵط�������uchar8 ��Щƽֻ̨����16λ��ģ�£�����32λ��ģ�¡�
�ڱ�̵�ʱ��Ҫ����ʶ���Լ���uchar8 uint16������32λ�ģ���һЩ255֮��ļӷ���Ȼ����������λ�����Ĳ����ͻ��в�ͬ��

*******************************************************************************/




#ifndef _DATATYPE_H
#define _DATATYPE_H		/* ��Ϊ��������ļ���������ļ�����ʱ������ͷ�ļ���Ҳ���������ļ���Ϊ��ȷ�����ظ����������Լ�һ������Ϊ��ǡ�
						   Ȼ������������ļ����ļ������һ�������ꡣ*/


#define _STM32F_TYPE		/* ���������ѡ��ͬ��mcuƽ̨ */



#ifdef _RENESAS_M16C_TYPE       /* Renesas ƽ̨ (M16C M30626FHPGP) */
typedef enum{
	             FALSE = 0,
				 TRUE = 1
			 }bool8;
typedef long int          int32;
typedef int               int16;
typedef char              char8;
typedef unsigned long int uint32;
typedef unsigned int      uint16;
typedef unsigned char     uchar8;
typedef float             float32;

#define BYTE_MASK         0xFF
#define BYTELENGTH        8
#define CHAR_BYTENUM      1
#define INT_BYTENUM       2
#define LONG_INT_BYTENUM  4
#define FLOAT_BYTENUM     4

#define FLOAT_MAX               4000000000
#define UNSIGNED_LONG_INT_MAX   4000000000
#define UNSIGNED_INT_MAX        65535
#define UNSIGNED_CHAR_MAX       255


#endif



#ifdef _MSP430_TYPE                   /* MSP430ƽ̨ */

typedef enum{
	             FALSE = 0,
				 TRUE = 1
			 }bool8;
typedef long int          int32;
typedef int               int16;
typedef char              char8;
typedef unsigned long int uint32;
typedef unsigned int      uint16;
typedef unsigned char     uchar8;
typedef float             float32;

#define BYTE_MASK         0xFF
#define BYTELENGTH        8
#define CHAR_BYTENUM      1
#define INT_BYTENUM       2
#define LONG_INT_BYTENUM  4
#define FLOAT_BYTENUM     4

#define FLOAT_MAX               4000000000
#define UNSIGNED_LONG_INT_MAX   4000000000
#define UNSIGNED_INT_MAX        65535
#define UNSIGNED_CHAR_MAX       255

#endif



#ifdef _STM32F_TYPE     /* STM32ƽ̨ */


typedef enum{
	            FALSE = 0,
				TRUE = 1
			 }bool8;
typedef signed long		int32;
typedef signed int		int16;
typedef signed char		char8;
typedef unsigned long		uint32;
typedef unsigned int		uint16;
typedef unsigned char		uchar8;
typedef unsigned long int	uint64;
typedef unsigned long long int uint128;
typedef signed long int	int64;
typedef signed long long int	int128;
typedef float			float32;

#define BYTE_MASK         0xFF
#define BYTELENGTH        8
#define CHAR_BYTENUM      1
#define INT_BYTENUM       2
#define LONG_INT_BYTENUM  4
#define FLOAT_BYTENUM     4

#define FLOAT_MAX               4000000000
#define UNSIGNED_LONG_INT_MAX   4000000000
#define UNSIGNED_INT_MAX        65535
#define UNSIGNED_CHAR_MAX       255

#endif






#endif























