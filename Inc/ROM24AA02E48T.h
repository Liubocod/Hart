/**
 ******************************************************************************
  * File Name          : ROM24AA02E48T.h

*/
#ifndef __ROM24AA02E48T_H
#define __ROM24AA02E48T_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"
#include "DataType.h"  

// ����ΪӲ���ӿڶ��壬����ʱ�ɸ��ݾ���I/O�ӿڽ��и���
#define		IICROM_SDA_Pin				GPIO_PIN_7
#define		IICROM_SDA_GPIO_Port			GPIOB

#define		IICROM_SCL_Pin				GPIO_PIN_6
#define		IICROM_SCL_GPIO_Port			GPIOB

#define		IICROM_SDA_H				HAL_GPIO_WritePin(IICROM_SDA_GPIO_Port, IICROM_SDA_Pin, GPIO_PIN_SET);
#define		IICROM_SDA_L				HAL_GPIO_WritePin(IICROM_SDA_GPIO_Port, IICROM_SDA_Pin, GPIO_PIN_RESET);

#define		IICROM_SCL_H				HAL_GPIO_WritePin(IICROM_SCL_GPIO_Port, IICROM_SCL_Pin, GPIO_PIN_SET);
#define		IICROM_SCL_L				HAL_GPIO_WritePin(IICROM_SCL_GPIO_Port, IICROM_SCL_Pin, GPIO_PIN_RESET);

#define		IICROM_DEVICE_ADDRESS			0xA0			// I2C�豸��ַ(����оƬ24AA02E48T���豸��ַ)��bit7~bit4�̶�Ϊ1010b
										// bit3~bit1ΪA2/A1/A0Ϊ�豸��ַλ��A2~A0��ƽ�ߵ�����Ӧ��Ӳ������ž���
										// 24AA02E48T����ֻ��VCC,VSS,SCL,SDA
#define		IICROM_DELAY_6US			0
#define		IICROM_DELAY_350US			200
#define		IICROM_DELAY_5MS			5
#define		IICROM_DELAY_1MS			1

#define  IICROM_TRY_TIME			3
   
#define		WRITE_PROTECT_START_ADDRESS		0x80
bool8	IICROM_LoadCharArrayData(uchar8 SaveAddress, uchar8 * Data, uint16 ArrayLength);
bool8	IICROM_SaveCharArrayData(uchar8 SaveAddress, uchar8 * Data, uint16 ArrayLength);
bool8	IICROM_SaveCharData(uchar8 SaveAddress, uchar8  Data);
void	IICRomDelay(uint16  DelayTime);

#ifdef __cplusplus
}
#endif
#endif /*__ROM24AA02E48T_H */
