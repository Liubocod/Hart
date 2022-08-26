/**
 ******************************************************************************
  * File Name          : LCD485.h

*/
#ifndef __LCD485_H
#define __LCD485_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"

#define		DEVICEADDRESS_INDEX				0
#define		MODBUSCOMMAND_INDEX				1

#define 	MAX_LCD485_BUFFER				200

#define 	LCD485WAITING					0
#define 	LCD485RECEIVING					1
#define 	LCD485READYTRANSMIT				2
#define 	LCD485TRANSMITING				3

#define 	LCD_485_RX_Pin					GPIO_PIN_10
#define 	LCD_485_RX_GPIO_Port				GPIOB
#define 	LCD_485_TX_Pin					GPIO_PIN_11
#define 	LCD_485_TX_GPIO_Port				GPIOB
#define 	LCD_485_DE_Pin					GPIO_PIN_12
#define		LCD_485_DE_GPIO_Port				GPIOB

#define 	EXPANDCHARPARA_STARTADDRESS_485			CHAREXPANDPARA_STARTA
#define 	EXPANDFLOATPARA_STARTADDRESS_485		FLOATEXPANDPARA_STARTA

#define		INTERVALTIME_MAX				50
void	EnableLCD485Receive(void);
void	LCD485_IRQHandler(void);
void	ModbusProtocol(void);
void	AutoTransmit(float TempDistance);

#ifdef __cplusplus
}
#endif
#endif /*__LCD485_H */
