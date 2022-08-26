#include 	"PMIC_CONTROL.h"
#include 	"main.h"
extern	I2C_HandleTypeDef hi2c3;
uint8_t         rxBuffer[47];

void	set_PMIC_to_PFM_mode(void)
{
	//CHANGE IWR PMIC MODE TO PFM
	uint8_t         txBuffer[2];
	uint8_t         txBufferBUCK2[2];
	uint8_t         txBufferBUCK3[2];

//	ENABLE_PMICEN1;
	//Check PGood
//	uint32_t PGoodStatus = 0;
/*	PGoodStatus = gpio_read_PGOOD();

	if(PGoodStatus)
	{
		uart_send_string("PGood Check Passed", UART_LF);
	}
	else
	{
		uart_send_string("PGood Check Failed", UART_LF);
	}*/
	/* Point to the PMIC and its power ready register */
//	txBuffer[0] = 0x20;
//	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBuffer,1,PMIC_LONG_TIMEOUT);
//	do{
//		HAL_I2C_Master_Receive(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)rxBuffer,1,PMIC_LONG_TIMEOUT);
//	} while(rxBuffer[0] != 0xCC);

//	txBuffer[0] = 0x01;
//	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBuffer,1,PMIC_LONG_TIMEOUT);
//	HAL_I2C_Master_Receive(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)rxBuffer,47,PMIC_LONG_TIMEOUT);
	
//	txBufferBUCK3[0] = 0x02; //BUCK1 control reg
//	txBufferBUCK3[1] = 0xC4; //BUCK1 set PFM auto only EN_BUCK0 controls 
//	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBufferBUCK3,2,PMIC_LONG_TIMEOUT);//Transmit BUCK3 settings

	txBufferBUCK3[0] = 0x04; //BUCK2 control reg
	txBufferBUCK3[1] = 0xC4; //BUCK2 set PFM auto
	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBufferBUCK3,2,PMIC_LONG_TIMEOUT);//Transmit BUCK3 settings

//	ENABLE_PMICEN1;
	txBuffer[0]      = 0x0E; //BUCK2_VOUT reg
	txBuffer[1]      = 0x89; //Set to 1.3V
	txBufferBUCK2[0] = 0x06; //BUCK2 control reg
	txBufferBUCK2[1] = 0xc4; //BUCK2 set PFM auto
	txBufferBUCK3[0] = 0x08; //BUCK3 control reg
	txBufferBUCK3[1] = 0xC4; //BUCK3 set PFM auto

	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBuffer,2,PMIC_LONG_TIMEOUT);//Transmit BUCK2_VOUT voltage
	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBufferBUCK2,2,PMIC_LONG_TIMEOUT);//Transmit BUCK2 settings
	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBufferBUCK3,2,PMIC_LONG_TIMEOUT);//Transmit BUCK3 settings
//	DISENABLE_PMICEN1;
		
	//UNCOMMENT FOR DEBUG PURPOSES, READS BACK BUCKS2+3
//	txBuffer[0] = 0x01;
//	HAL_I2C_Master_Transmit(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)txBuffer,1,PMIC_LONG_TIMEOUT);
//	HAL_I2C_Master_Receive(&hi2c3, (uint16_t)PMIC_ADDRESS, (uint8_t *)rxBuffer,47,PMIC_LONG_TIMEOUT);
//	ENABLE_PMICEN1;
}
