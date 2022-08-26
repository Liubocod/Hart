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




/***********************DataType.h文件说明**************************************
本文件主要是根据不同的mcu的种类型号来将常用的数据类型予以对应。

主要用的数据类型有uchar8、uint16、char8、int16、float32

其它文件中的数据类型尽量用上面这四种。
然后在不同的mcu平台通过本文件来与具体的mcu提供的数据类型对应。
主要不一样的地方往往是uchar8 有些平台只能用16位来模仿，或者32位来模仿。
在编程的时候要有意识：自己的uchar8 uint16可能是32位的，做一些255之后的加法，然后再向右移位这样的操作就会有不同。

*******************************************************************************/




#ifndef _DATATYPE_H
#define _DATATYPE_H		/* 因为会有许多文件包含这个文件，有时甚至于头文件中也会包含这个文件，为了确保不重复包含，所以加一个宏作为标记。
						   然后在引用这个文件的文件里面加一个条件宏。*/


#define _STM32F_TYPE		/* 本条语句来选择不同的mcu平台 */



#ifdef _RENESAS_M16C_TYPE       /* Renesas 平台 (M16C M30626FHPGP) */
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



#ifdef _MSP430_TYPE                   /* MSP430平台 */

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



#ifdef _STM32F_TYPE     /* STM32平台 */


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























