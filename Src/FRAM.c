/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "gpio.h"
#include "FRAM.h"

uint8_t DeviceID_FRAM[9];
uint8_t StatusRegister_FRAM;
uint8_t	FRAM_Address_Numbers_Max;
MeterWorkTotalTime___TypeDef MeterWorkTotalTime;

HAL_StatusTypeDef WriteCommand_FRAM(uint8_t TempCommand)
{
	return(HAL_SPI_Transmit(&hspi2, &TempCommand, 1, FRAM_TIMEOUT));
}

HAL_StatusTypeDef ReadDeviceID(void)
{
	HAL_StatusTypeDef Status;
	ENABLE_FRAM_CS;
	Status = WriteCommand_FRAM(RDID_OPCODE);
	if(Status == HAL_OK)
	{
		Status = HAL_SPI_Receive(&hspi2, &DeviceID_FRAM[0],9, FRAM_TIMEOUT);
	}
	DISABLE_FRAM_CS;
	return(Status);
}

HAL_StatusTypeDef ReadStatusRegister(void)
{
	HAL_StatusTypeDef Status;
	ENABLE_FRAM_CS;
	Status = WriteCommand_FRAM(RDSR_OPCODE);
	if(Status == HAL_OK)
	{
		Status = HAL_SPI_Receive(&hspi2, &StatusRegister_FRAM,1, FRAM_TIMEOUT);
	}
	DISABLE_FRAM_CS;
	return(Status);
}

HAL_StatusTypeDef WriteFRAM(uint32_t Address_FRAM,uint16_t TempNumberByte_FRAM,uint8_t *WriteData_FRAM)
{
	uint8_t TempAddress_FRAM[3];
	HAL_StatusTypeDef Status;
	
	if(FRAM_Address_Numbers_Max == 3)
	{
		TempAddress_FRAM[2] = (uint8_t)(Address_FRAM);
		TempAddress_FRAM[1] = (uint8_t)(Address_FRAM>>8);
		TempAddress_FRAM[0] = (uint8_t)(Address_FRAM>>16);
	}
	else if(FRAM_Address_Numbers_Max == 2)
	{
		TempAddress_FRAM[1] = (uint8_t)(Address_FRAM);
		TempAddress_FRAM[0] = (uint8_t)(Address_FRAM>>8);
	}
	ENABLE_FRAM_CS;
	Status = WriteCommand_FRAM(WREN_OPCODE);
	DISABLE_FRAM_CS;
	if(Status == HAL_OK)
	{
		ENABLE_FRAM_CS;
		Status = WriteCommand_FRAM(WRITE_OPCODE);
		if(Status == HAL_OK)
		{
			HAL_SPI_Transmit(&hspi2, &TempAddress_FRAM[0],FRAM_Address_Numbers_Max, FRAM_TIMEOUT);
			Status = HAL_SPI_Transmit(&hspi2, WriteData_FRAM,TempNumberByte_FRAM, FRAM_TIMEOUT);
		}
		DISABLE_FRAM_CS;
	}
	ENABLE_FRAM_CS;
	Status = WriteCommand_FRAM(WRDI_OPCODE);
	DISABLE_FRAM_CS;
	return(Status);
}

HAL_StatusTypeDef ReadFRAM(uint32_t Address_FRAM,uint16_t TempNumberByte_FRAM,uint8_t *ReadData_FRAM)
{
	uint8_t TempAddress_FRAM[3];
	HAL_StatusTypeDef Status;
	
	if(FRAM_Address_Numbers_Max == 3)
	{
		TempAddress_FRAM[2] = (uint8_t)(Address_FRAM);
		TempAddress_FRAM[1] = (uint8_t)(Address_FRAM>>8);
		TempAddress_FRAM[0] = (uint8_t)(Address_FRAM>>16);
	}
	else if(FRAM_Address_Numbers_Max == 2)
	{
		TempAddress_FRAM[1] = (uint8_t)(Address_FRAM);
		TempAddress_FRAM[0] = (uint8_t)(Address_FRAM>>8);
	}
	ENABLE_FRAM_CS;
	Status = WriteCommand_FRAM(READ_OPCODE);
	if(Status == HAL_OK)
	{
		Status = HAL_SPI_Transmit(&hspi2, &TempAddress_FRAM[0],FRAM_Address_Numbers_Max, FRAM_TIMEOUT);
		if(Status == HAL_OK)
		{
			Status = HAL_SPI_Receive(&hspi2, ReadData_FRAM,TempNumberByte_FRAM, FRAM_TIMEOUT);
		}
	}
	DISABLE_FRAM_CS;
	return(Status);
}

float	ReadFloatToFRAM(uint32_t TempParaAddress)
{
//	MeterWorkTotalTime.MeterWorkTotalTime_Float[0] = TempFloat;
	ReadFRAM(TempParaAddress,4,&MeterWorkTotalTime.MeterWorkTotalTime_Char[0]);
	return(MeterWorkTotalTime.MeterWorkTotalTime_Float[0]);
}

void	InitFRAM(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = FRAM_WP_Pin|FRAM_NSS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(FRAM_WP_GPIO_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = FRAM_HOLD_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(FRAM_HOLD_GPIO_Port, &GPIO_InitStruct);
	
	DISABLE_FRAM_CS;
	ENABLE_FRAM_HOLD;
	DISABLE_FRAM_WP;
	ReadDeviceID();
	if((DeviceID_FRAM[6] == 0xC2)&&(DeviceID_FRAM[7] == 0x24))
	{
		FRAM_Address_Numbers_Max = 3;
	}
	else
	{
		FRAM_Address_Numbers_Max = 2;
	}
	ReadStatusRegister();
	
//	WriteFRAM(0x0004,3,&MeterWorkTotalTime.MeterWorkTotalTime_Char[0]);
//	ReadFRAM(12,8,&MeterWorkTotalTime.MeterWorkTotalTime_Char[0]);
}
