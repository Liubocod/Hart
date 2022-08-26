/**
 ******************************************************************************
  * File Name          : IWR1443.h

*/
#ifndef __IWR1443_H
#define __IWR1443_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"

//#define		IWR1443DATA_STATUS

#define		MAXIWR1443WORKTIME_FM				230	//ms
#define		MAXIWR1443WORKTIME_PM				230	//ms
#define 	STARTIWR1443UART				45//45	//ms
#define 	TRANSMITTOIWR1443				38	//ms
#define 	MAXIWR1443WORKPERIOD				3000	//ms
#define 	IWR1443_WAIT_STATUS				0
#define 	IWR1443_START_STATUS				1
#define 	IWR1443_CONFIGING_STATUS			2
#define 	IWR1443_CONFIGEND_STATUS			3
#define 	IWR1443_WORKING_STATUS				4
#define 	IWR1443_START_POWERON				0x5A

#define 	IWR1443_RDATA_TIMROUT				2
#define 	IWR1443_RDATA_UNUSE				1
#define 	IWR1443_RDATA_USED				0
#define 	IWR1443_WAVEDATA_LENGTH				100

#define 	TRANSMIT_MAXBYTES				30
#define		MAXCHIPNUMBERS					5

#define		CON_RF_POWER_12_Pin				GPIO_PIN_13
#define 	CON_RF_POWER_12_GPIO_Port			GPIOD
#define 	CON_RF_POWER_13_Pin				GPIO_PIN_12
#define 	CON_RF_POWER_13_GPIO_Port			GPIOD
#define 	CON_RF_POWER_18_Pin				GPIO_PIN_10
#define 	CON_RF_POWER_18_GPIO_Port			GPIOD
#define 	CON_RF_POWER_22_Pin				GPIO_PIN_11
#define 	CON_RF_POWER_22_GPIO_Port			GPIOD
#define 	CON_RF_POWER_33_Pin				GPIO_PIN_14
#define 	CON_RF_POWER_33_GPIO_Port			GPIOD
#define 	IRW1443_RST_Pin					GPIO_PIN_9
#define 	IRW1443_RST_GPIO_Port				GPIOC

#define		ENABLE_RF_VCC33					HAL_GPIO_WritePin(CON_RF_POWER_33_GPIO_Port, CON_RF_POWER_33_Pin, GPIO_PIN_SET);
#define		DISABLE_RF_VCC33				HAL_GPIO_WritePin(CON_RF_POWER_33_GPIO_Port, CON_RF_POWER_33_Pin, GPIO_PIN_RESET);

#define		ENABLE_RF_VCC12					HAL_GPIO_WritePin(CON_RF_POWER_12_GPIO_Port, CON_RF_POWER_12_Pin, GPIO_PIN_SET);
#define		DISABLE_RF_VCC12				HAL_GPIO_WritePin(CON_RF_POWER_12_GPIO_Port, CON_RF_POWER_12_Pin, GPIO_PIN_RESET);

#define		ENABLE_RF_VCC13					HAL_GPIO_WritePin(CON_RF_POWER_13_GPIO_Port, CON_RF_POWER_13_Pin, GPIO_PIN_SET);
#define		DISABLE_RF_VCC13				HAL_GPIO_WritePin(CON_RF_POWER_13_GPIO_Port, CON_RF_POWER_13_Pin, GPIO_PIN_RESET);
	
#define		ENABLE_RF_VCC18					HAL_GPIO_WritePin(CON_RF_POWER_18_GPIO_Port, CON_RF_POWER_18_Pin, GPIO_PIN_SET);
#define		DISABLE_RF_VCC18				HAL_GPIO_WritePin(CON_RF_POWER_18_GPIO_Port, CON_RF_POWER_18_Pin, GPIO_PIN_RESET);

#define		ENABLE_RF_VCC22					HAL_GPIO_WritePin(CON_RF_POWER_22_GPIO_Port, CON_RF_POWER_22_Pin, GPIO_PIN_SET);
#define		DISABLE_RF_VCC22				HAL_GPIO_WritePin(CON_RF_POWER_22_GPIO_Port, CON_RF_POWER_22_Pin, GPIO_PIN_RESET);

#define		Enable_RF_NRST					HAL_GPIO_WritePin(IRW1443_RST_GPIO_Port, IRW1443_RST_Pin, GPIO_PIN_RESET);
#define		Disable_RF_NRST					HAL_GPIO_WritePin(IRW1443_RST_GPIO_Port, IRW1443_RST_Pin, GPIO_PIN_SET);

#define		PARTCALIB_MODE					1
#define		FULLCALIB_MODE					0
  
#define		DRECEIVE_START					0

#ifdef	IWR1443DATA_STATUS
	#define		RECEIVEBUFFERSIZE_80G			206//188//
	#define		RECEIVEBUFFER_CURVEADDRESS		105//87//
	#define		RECEIVEBUFFER_CHIPADDRESS		54//18//
	#define		RECEIVEBUFFER_LEADCODEADDRESS		24//6//
//	#define		RECEIVEBUFFERSIZE_80G			182//+24
//	#define		RECEIVEBUFFER_CURVEADDRESS		81//+24
//	#define		RECEIVEBUFFER_CHIPADDRESS		6//+48
//	#define		RECEIVEBUFFER_LEADCODEADDRESS		0//+24
	#define		RECEIVEBUFFERSIZE_CRC			358
#else
	#define		RECEIVEBUFFERSIZE_80G			DRECEIVE_START+182//206//188//
	#define		RECEIVEBUFFER_CURVEADDRESS		DRECEIVE_START+82//106//87//
	#define		RECEIVEBUFFER_CHIPADDRESS		DRECEIVE_START*2+8//56//18//
	#define		RECEIVEBUFFER_LEADCODEADDRESS		DRECEIVE_START//24//6//
	#define		RECEIVEBUFFERSIZE_CRC			360
#endif

#define		RECEIVEBUFFER_VERSION				RECEIVEBUFFER_LEADCODEADDRESS+2//188//
#define		RECEIVEBUFFERSIZE_80G_512FFT			RECEIVEBUFFERSIZE_80G+512//188//
#define		RECEIVEBUFFERSIZE_80G_1024FFT			RECEIVEBUFFERSIZE_80G+1024//188//
#define		RECEIVEBUFFERSIZE_STATUS			2

typedef	union
{
	uint8_t	RxDataBufferC[RECEIVEBUFFERSIZE_80G_1024FFT*2];
	uint16_t RxDataBufferI[RECEIVEBUFFERSIZE_80G_1024FFT];
}ReceiveDataBuffer__TypeDef;

void	HandleIWR1443_ControlFlow(void);
void	IncIWR443_WorkTime(void);
void	InitIWR1443Para(void);
void	Set_IWR1443_Status(uint8_t CurrentStatus);
uint8_t	Get_IWR1443_Status(void);
void	EndIWR1443Work(void);
void	StartTransmitToIWR1443(void);
void	DisableIWR1443Vcc(void);
void	AbnormalCloseWR1443Vcc(void);
float	GetIWR1443WorkPeriod(void);
uint16_t Get_IWR1443_Version(void);
uint8_t	GetIWR1443_WorkMode(void);
void	ClearIWR1443UnWorkCount(void);

#ifdef __cplusplus
}
#endif
#endif /*__IWR1443_H */
