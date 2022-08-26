/**
 ******************************************************************************
  * File Name          : BlueTooth_Com.h

*/
#ifndef __BlueTooth_Com_H
#define __BlueTooth_Com_H
#ifdef __cplusplus
 extern "C" {
#endif

   
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "main.h"

#define		DEVICEADDRESS_INDEX				0
#define		MODBUSCOMMAND_INDEX				1

#define 	MAX_BT_BUFFER					200

#define 	BTWAITING					0
#define 	BTRECEIVING					1
#define 	BTREADYTRANSMIT					2
#define 	BTTRANSMITING					3

#define		UPDATEADVDATA					1

#define		POWERON_BLTOOTH					0
#define		SETADV_DATA_BLTOOTH				1
#define		CONNECTED_BLTOOTH				2
#define		SENDUART_DATA_BLTOOTH				3
#define		DISCONNECTED_BLTOOTH				4

#define 	BT_RX_Pin					GPIO_PIN_11
#define 	BT_RX_GPIO_Port					GPIOC
#define 	BT_TX_Pin					GPIO_PIN_10
#define 	BT_TX_GPIO_Port					GPIOC

#define 	BT_STATUS_Pin					GPIO_PIN_8
#define 	BT_STATUS_GPIO_Port				GPIOC
#define 	BT_RESET_Pin					GPIO_PIN_15
#define 	BT_RESET_GPIO_Port				GPIOD

#define 	EXPANDCHARPARA_STARTADDRESS_485			CHAREXPANDPARA_STARTA
#define 	EXPANDFLOATPARA_STARTADDRESS_485		FLOATEXPANDPARA_STARTA

#define		INTERVALTIME_MAX				50
#define		BLTOOTH_STATUS_MAXTIME				60000
#define		BLTOOTH_STATUS_L_MAXTIME			8000
#define		BLTOOTH_STATUS_H_MAXTIME			1500

#define		DISABLE_RESET_BLTOOTH				HAL_GPIO_WritePin(BT_RESET_GPIO_Port, BT_RESET_Pin, GPIO_PIN_SET);
#define		ENABLE_RESET_BLTOOTH				HAL_GPIO_WritePin(BT_RESET_GPIO_Port, BT_RESET_Pin, GPIO_PIN_RESET);

#define		ENABLE_VCC_BLTOOTH				HAL_GPIO_WritePin(BT_VCC_GPIO_Port, BT_VCC_Pin, GPIO_PIN_SET);
#define		DISABLE_VCC_BLTOOTH				HAL_GPIO_WritePin(BT_VCC_GPIO_Port, BT_VCC_Pin, GPIO_PIN_RESET);
void	EnableBTReceive(void);
void	BT_IRQHandler(void);
void	BT_ModbusProtocol(void);
void	ReadBLTooth_Status(void);
void	Set_BLT_Adv_Data(void);
void	Set_BLTooth_Adv_Data(uint8_t CurrenStatus);
void	SetProductType_Int(uint16_t Temp_ProductType);
void	SetProductSL_Int(uint16_t Temp_ProductSL);
uint8_t	GetBlueToothStatus(void);

#ifdef __cplusplus
}
#endif
#endif /*__BlueTooth_Com_H */
