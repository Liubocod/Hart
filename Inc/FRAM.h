/**
 ******************************************************************************
  * File Name          : FM25V10.h

*/
#ifndef __FM25V10_H
#define __FM25V10_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"

#define		WREN_OPCODE				0x06
#define 	WRDI_OPCODE				0x04
#define 	WRITE_OPCODE				0x02
#define 	READ_OPCODE				0x03
#define 	RDSR_OPCODE				0x05
#define 	RDID_OPCODE				0x9F
#define 	SLEEP_OPCODE				0xB9

#define 	FRAM_ADDRESS_NUMBER			3

#define		FRAM_NSS_Pin				GPIO_PIN_0
#define 	FRAM_NSS_GPIO_Port			GPIOD
#define 	FRAM_SCK_Pin				GPIO_PIN_1
#define 	FRAM_SCK_GPIO_Port			GPIOD
#define 	FRAM_MISO_Pin				GPIO_PIN_3
#define 	FRAM_MISO_GPIO_Port			GPIOD
#define 	FRAM_MOSI_Pin				GPIO_PIN_4
#define 	FRAM_MOSI_GPIO_Port			GPIOD
#define 	FRAM_WP_Pin				GPIO_PIN_2
#define 	FRAM_WP_GPIO_Port			GPIOD
#define 	FRAM_HOLD_Pin				GPIO_PIN_12
#define 	FRAM_HOLD_GPIO_Port			GPIOC

#define 	FRAM_TIMEOUT				300//mS

#define		ENABLE_FRAM_HOLD			HAL_GPIO_WritePin(FRAM_HOLD_GPIO_Port, FRAM_HOLD_Pin, GPIO_PIN_SET);
#define		DISABLE_FRAM_WP				HAL_GPIO_WritePin(FRAM_WP_GPIO_Port, FRAM_WP_Pin, GPIO_PIN_SET);
#define		ENABLE_FRAM_CS				HAL_GPIO_WritePin(FRAM_NSS_GPIO_Port, FRAM_NSS_Pin, GPIO_PIN_RESET);
#define		DISABLE_FRAM_CS				HAL_GPIO_WritePin(FRAM_NSS_GPIO_Port, FRAM_NSS_Pin, GPIO_PIN_SET);

typedef	union
{
	uint8_t	MeterWorkTotalTime_Char[8];
	uint32_t MeterWorkTotalTime_Long[2];
	float MeterWorkTotalTime_Float[2];
}MeterWorkTotalTime___TypeDef;

void	InitFRAM(void);
HAL_StatusTypeDef WriteFRAM(uint32_t Address_FRAM,uint16_t TempNumberByte_FRAM,uint8_t *WriteData_FRAM);
HAL_StatusTypeDef ReadFRAM(uint32_t Address_FRAM,uint16_t TempNumberByte_FRAM,uint8_t *ReadData_FRAM);

#ifdef __cplusplus
}
#endif
#endif /*__FM25V10_H */
