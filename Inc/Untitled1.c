#include	"main.h"
#include	"usart.h"
#include	"Eeprom.h"
#include	"LCD485.h"

uint8_t ReceiveBuffer_LCD485[MAX_LCD485_BUFFER];
uint8_t TxRxCount_LCD485,ReceiveDataNumber_ThisTime;
uint8_t ReceiveFlage_LCD485,TransmitFlage_LCD485;
/*--------------------------MODBUS CRCº∆À„----------------------------------------------*/
uint16_t	crc_check(unsigned char pack_byte)
{
	unsigned int polyn_m = 0xA001;
	unsigned char crc_i,crc_j;
	unsigned int crc = 0xFFFF;
	for(crc_i=0;crc_i<pack_byte;crc_i++)
	{
		crc ^= ReceiveBuffer_LCD485[crc_i];
		for(crc_j=0;crc_j<8;crc_j++)
		{
			if((crc&0x01)==0x01)
			{
				crc>>=1;
				crc^=polyn_m;
			}
			else
				crc>>=1;
		}
	}
	return( crc );
}

void	EnableLCD485Receive(void)
{
	HAL_UART_Receive_IT(&hlpuart1, (uint8_t *)ReceiveBuffer_LCD485, 20);
}

void	ModbusProtocol(void)
{
	
}

static	HAL_StatusTypeDef LCD485_Receive_IT(void)
{
	uint8_t  TempC,TempAddress;
	uint16_t  uhdata;

	/* Check that a Rx process is ongoing */
	if(hlpuart1.RxState == HAL_UART_STATE_BUSY_RX)
	{
		uhdata = (uint16_t) READ_REG(hlpuart1.Instance->RDR)&0x00FF;
		
		if((uhdata == GetComAddress())&&(uhdata == 0xAA))
		{
			ReceiveFlage_LCD485 = 1;
			ReceiveBuffer_LCD485[0] = UartTemp;
			TxRxCount_LCD485 = 0;
		}
		else
		{
			TxRxCount_LCD485++;
			ReceiveBuffer_LCD485[TxRxCount_LCD485] = UartTemp;
			if(TxRxCount_LCD485 < 7)
			{
				if((ReceiveBuffer_LCD485[1] <= 0x05)&&(ReceiveBuffer_LCD485[1] >= 0x01))
				{
					ReceiveDataNumber_ThisTime = 7;
				}
				else
				{
					if(ReceiveBuffer_LCD485[1] == 0x10)   //person computer write
					{
						if(TxRxCount_LCD485 == 6)
						{
							ReceiveDataNumber_ThisTime = ReceiveBuffer_LCD485[6]+8;
						}
					}
					else
					{
						ReceiveFlage_LCD485 = 0;
						TxRxCount_LCD485 = 0;
					}
				}
			}
			else
			{
				if(TxRxCount_LCD485 == ReceiveDataNumber_ThisTime)
				{
					ReceiveFlage_LCD485 = 0;
					TransmitFlage_LCD485 = 1;
				}
			}
		}
		return HAL_OK;
	}
	else
	{
		/* Clear RXNE interrupt flag */
		__HAL_UART_SEND_REQ(&hlpuart1, UART_RXDATA_FLUSH_REQUEST);
		return HAL_BUSY;
	}
}

static HAL_StatusTypeDef LCD485_Transmit_IT()
{
	/* Check that a Tx process is ongoing */
	if (hlpuart1.gState == HAL_UART_STATE_BUSY_TX)
	{
		if(TxRxCount_LCD485 > TxNumber)
		{
			/* Disable the UART Transmit Data Register Empty Interrupt */
			CLEAR_BIT(hlpuart1.Instance->CR1, USART_CR1_TXEIE);
			/* Enable the UART Transmit Complete Interrupt */
			SET_BIT(hlpuart1.Instance->CR1, USART_CR1_TCIE);
			return HAL_OK;
		}
		else
		{
			hlpuart1.Instance->TDR = (uint8_t)(HartDataBuffer[TxRxCount]);
			TxRxCount++;
			return HAL_OK;
		}
	}
	else
	{
		return HAL_BUSY;
	}
}

void	LCD485_IRQHandler(void)
{
	uint32_t isrflags   = READ_REG(hlpuart1.Instance->ISR);
	uint32_t cr1its     = READ_REG(hlpuart1.Instance->CR1);
	uint32_t cr3its;
	uint32_t errorflags;

	/* If no error occurs */
	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
	if (errorflags == RESET)
	{
		/* UART in mode Receiver ---------------------------------------------------*/
		if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{
			LCD485_Receive_IT();
			return;
		}
	}  

	/* If some errors occur */
	cr3its = READ_REG(hlpuart1.Instance->CR3);
	if( (errorflags != RESET)&& ( ((cr3its & USART_CR3_EIE) != RESET)|| ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)) )
	{
		/* UART parity error interrupt occurred -------------------------------------*/
		if(((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_PEF);
			huart1.ErrorCode |= HAL_UART_ERROR_PE;
		}

		/* UART frame error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_FEF);

			hlpuart1.ErrorCode |= HAL_UART_ERROR_FE;
		}

		/* UART noise error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_NEF);
			hlpuart1.ErrorCode |= HAL_UART_ERROR_NE;
		}
    
		/* UART Over-Run interrupt occurred -----------------------------------------*/
		if(((isrflags & USART_ISR_ORE) != RESET)&&(((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET)))
		{
			__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_OREF);
			hlpuart1.ErrorCode |= HAL_UART_ERROR_ORE;
		}

		/* Call UART Error Call back function if need be --------------------------*/
		if(hlpuart1.ErrorCode != HAL_UART_ERROR_NONE)
		{
			/* UART in mode Receiver ---------------------------------------------------*/
			if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
			{
				LCD485_Receive_IT();
			}
		}
		return;
	} /* End if some error occurs */

	/* UART wakeup from Stop mode interrupt occurred ---------------------------*/
	cr3its = READ_REG(huart1.Instance->CR3);
	if(((isrflags & USART_ISR_WUF) != RESET) && ((cr3its & USART_CR3_WUFIE) != RESET))
	{
		__HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_WUF);
		/* Set the UART state ready to be able to start again the process */
		hlpuart1.gState  = HAL_UART_STATE_READY;
		hlpuart1.RxState = HAL_UART_STATE_READY;
		HAL_UARTEx_WakeupCallback(&hlpuart1);
		return;
	}

	/* UART in mode Transmitter ------------------------------------------------*/
	if(((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
	{
		LCD485_Transmit_IT();
		return;
	}

	/* UART in mode Transmitter (transmission end) -----------------------------*/
	if(((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
//		UART_EndTransmit_IT(&hlpuart1);
		/* Disable the UART Transmit Complete Interrupt */
		CLEAR_BIT(hlpuart1.Instance->CR1, USART_CR1_TCIE);
		/* Tx process is ended, restore huart->gState to Ready */
		huart1.gState = HAL_UART_STATE_READY;
		HAL_UART_TxCpltCallback(&hlpuart1);
		EnableLCD485Receive();
		return;
	}
}
