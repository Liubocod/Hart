#include	"main.h"
#include	"usart.h"
#include	"Eeprom.h"
#include 	"HartHandle.h"
#include 	"DistanceDataHandle.h"

uint8_t	HartDataBuffer[30],DataFieldStartAddress;
uint8_t TxRxCount,TxNumber,DelimiterFlag,FrameByteNuber,RFlag;

//---------------------------------------------------------------------------------
uint8_t	ReckonHartCRC(uint8_t CRCNumber)
{
	uint8_t TempC,CheckCode;
	CheckCode = 0xFF;
	for(TempC = 0; TempC < CRCNumber; TempC++)
	{
		CheckCode ^= HartDataBuffer[TempC];
	}
	return(~CheckCode);
}

void	EnableHartRx(void)
{
	ENABLE_HART_RX;
	RFlag = HART_RFRAME_FAIL;
	DelimiterFlag = 0;
	TxRxCount = 0;
	FrameByteNuber = 50;
	HAL_UART_Receive_IT(&huart1, (uint8_t *)HartDataBuffer, RXBUFFERSIZE);
}

void	EnableHartTx(void)
{
	uint8_t TempC;
	ENABLE_HART_TX;
	HartDataBuffer[TxNumber] = ReckonHartCRC(TxNumber);
	for(TempC = TxNumber; TempC > 0; TempC--)
	{
		HartDataBuffer[TempC+DEFAULT_PREAM] = HartDataBuffer[TempC];
	}
	HartDataBuffer[DEFAULT_PREAM] = HartDataBuffer[0];
	for(TempC = 0; TempC < DEFAULT_PREAM; TempC++)
		HartDataBuffer[TempC] = 0xFF;
	TxNumber = TxNumber+DEFAULT_PREAM;
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)HartDataBuffer, TxNumber);
}

void	FloatToTFrameChar(float FloatVar,uint8_t DataIndex)
{
	uint8_t *point_Float;
	point_Float = (uint8_t *)&FloatVar;
	
	HartDataBuffer[DataIndex] = point_Float[3];
	HartDataBuffer[DataIndex+1] = point_Float[2];
	HartDataBuffer[DataIndex+2] = point_Float[1];
	HartDataBuffer[DataIndex+3] = point_Float[0];	
}

float	RFrameCharToFloat (uint8_t DataIndex )//tranfer 4 char variables at the proper location of the struct Hart_TFrame into 1 float variable 
{    
	float *FloatVar;
	uint8_t point_Float[4];
	point_Float[3] = HartDataBuffer[DataIndex];
	point_Float[2] = HartDataBuffer[DataIndex+1];
	point_Float[1] = HartDataBuffer[DataIndex+2];
	point_Float[0] = HartDataBuffer[DataIndex+3];
	FloatVar = ( float* )point_Float;
	return *FloatVar;
}

void	HARTCMD0(void)//READ UNIQUE IDENTIFIER
{
	HartDataBuffer[DataFieldStartAddress+1] = 14;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;
	HartDataBuffer[DataFieldStartAddress+4] = 0xFE;
	HartDataBuffer[DataFieldStartAddress+5] = MANUFACTURE_ID;
	HartDataBuffer[DataFieldStartAddress+6] = DEVICE_TYPE;
	HartDataBuffer[DataFieldStartAddress+7] = DEFAULT_PREAM;
	HartDataBuffer[DataFieldStartAddress+8] = UNIV_CMD_REV;
	HartDataBuffer[DataFieldStartAddress+9] = TRANS_SPEC_REV;
	HartDataBuffer[DataFieldStartAddress+10] = SOFT_REV;
	HartDataBuffer[DataFieldStartAddress+11] = HARD_REV;
	HartDataBuffer[DataFieldStartAddress+12] = FLAGS;
	HartDataBuffer[DataFieldStartAddress+13] = DEVICE_ID0;
	HartDataBuffer[DataFieldStartAddress+14] = DEVICE_ID1;
	HartDataBuffer[DataFieldStartAddress+15] = DEVICE_ID2|GetComAddress();
	TxNumber = DataFieldStartAddress+16;
}

void	HARTCMD1(void)//READ PRIMARY VARIABLE
{
	HartDataBuffer[DataFieldStartAddress+1] = 7;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	HartDataBuffer[DataFieldStartAddress+4] = 0x2d;//HART_Variables.PVUnit;
	FloatToTFrameChar(HartReadMasterVariable(0),DataFieldStartAddress+5);
	TxNumber = DataFieldStartAddress+9;
}

void	HARTCMD2(void)//READ P. V. CURRENT AND PERCENT OF RANGE
{
	HartDataBuffer[DataFieldStartAddress+1] = 10;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	FloatToTFrameChar(HartReadMasterVariable(3),DataFieldStartAddress+4);
	FloatToTFrameChar(HartReadMasterVariable(2),DataFieldStartAddress+8);
	TxNumber = DataFieldStartAddress+12;
}

void	HARTCMD3(void)//READ DYNAMIC VARIABLES AND P. V. CURRENT
{
	HartDataBuffer[DataFieldStartAddress+1] = 26;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	FloatToTFrameChar(HartReadMasterVariable(3),DataFieldStartAddress+4);
	HartDataBuffer[DataFieldStartAddress+8] = 0x2D;
	FloatToTFrameChar(HartReadMasterVariable(1),DataFieldStartAddress+9);
	HartDataBuffer[DataFieldStartAddress+13] = 0x2D;
	FloatToTFrameChar(0,DataFieldStartAddress+14);
	HartDataBuffer[18] = 0x2D;
	FloatToTFrameChar(0,DataFieldStartAddress+19);
	HartDataBuffer[23] = 0x2D;
	FloatToTFrameChar(0,DataFieldStartAddress+24);
	TxNumber = DataFieldStartAddress+25;
}

void	HARTCMD6(void)//address
{
	SavePara_CharType(17,HartDataBuffer[DataFieldStartAddress+2]);
	HartDataBuffer[DataFieldStartAddress+1] = 3;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;
	ReadParaC_LCD(17,&HartDataBuffer[DataFieldStartAddress+4],1);
	TxNumber = DataFieldStartAddress+5;
}

void	HARTCMD12(void)//read message
{
	HartDataBuffer[DataFieldStartAddress+1] = 26;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	ReadParaC_LCD(31, &HartDataBuffer[DataFieldStartAddress+4],6);
	ReadParaC_LCD(29, &HartDataBuffer[DataFieldStartAddress+10],10);
	ReadParaC_LCD(28, &HartDataBuffer[DataFieldStartAddress+20],8);
	TxNumber = DataFieldStartAddress+29;
}

void	HARTCMD13(void)//read message
{
	HartDataBuffer[DataFieldStartAddress+1] = 26;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	ReadParaC_LCD(31, &HartDataBuffer[DataFieldStartAddress+4],6);
	ReadParaC_LCD(30, &HartDataBuffer[DataFieldStartAddress+10],12);
	ReadParaC_LCD(28, &HartDataBuffer[DataFieldStartAddress+22],3);
	TxNumber = DataFieldStartAddress+23;
}

void	HARTCMD14(void)//READ PRIMARY VARIABLE SENSOR INFORMATION
{
	HartDataBuffer[DataFieldStartAddress+1] = 16;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	ReadParaC_LCD(29, &HartDataBuffer[DataFieldStartAddress+4],3);
	HartDataBuffer[DataFieldStartAddress+5] = 0x2d;//HART_Variables.PVUnit;
	ReadParaF_Hart(41, &HartDataBuffer[DataFieldStartAddress+6]);
	ReadParaF_Hart(40, &HartDataBuffer[DataFieldStartAddress+10]);
	FloatToTFrameChar(0.001,DataFieldStartAddress+14);
	TxNumber = DataFieldStartAddress+18;
}

void	HARTCMD15(void)//READ PRIMARY VARIABLE OUTPUT INFORMATION
{
	HartDataBuffer[DataFieldStartAddress+1] = 19;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	HartDataBuffer[DataFieldStartAddress+4] = 0x2d;//alarm slect
	HartDataBuffer[DataFieldStartAddress+5] = 0x2d;//Unit;
	HartDataBuffer[DataFieldStartAddress+6] = 0x2d;//Unit;
	ReadParaF_Hart(35, &HartDataBuffer[DataFieldStartAddress+7]);
	ReadParaF_Hart(34, &HartDataBuffer[DataFieldStartAddress+11]);
	ReadParaC_LCD(11, &HartDataBuffer[DataFieldStartAddress+15],1);
	FloatToTFrameChar((float)(HartDataBuffer[DataFieldStartAddress+12]),DataFieldStartAddress+15);
	HartDataBuffer[DataFieldStartAddress+19] = 0x2d;//WriteProtectCode
	HartDataBuffer[DataFieldStartAddress+20] = 0x2d;//PVLableDistCode;
	TxNumber = DataFieldStartAddress+21;
}

void	HARTCMD16(void)//READ FINAL ASSEMBLY NUMBER
{
	HartDataBuffer[DataFieldStartAddress+1] = 5;
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;

	ReadParaC_LCD(32, &HartDataBuffer[DataFieldStartAddress+4],3);
	TxNumber = DataFieldStartAddress+7;
}

void	HARTCMD18(void)//WRITE TAG, DESCRIPTOR, DATE
{
	uint8_t TempC;
	HartDataBuffer[DataFieldStartAddress+1] = HartDataBuffer[DataFieldStartAddress+1]+2;

	SavePara_StringType(31,&HartDataBuffer[DataFieldStartAddress+2]);
	SavePara_StringType(30,&HartDataBuffer[DataFieldStartAddress+8]);
	SavePara_StringType(28,&HartDataBuffer[DataFieldStartAddress+20]);
	for(TempC = FrameByteNuber; TempC >= (DataFieldStartAddress+2); TempC++)
	{
		HartDataBuffer[TempC+2] = HartDataBuffer[TempC];
	}
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;
	TxNumber = FrameByteNuber+2;
}

void	HARTCMD19(void)//WRITE FINAL ASSEMBLY NUMBER
{
	uint8_t TempC;
	HartDataBuffer[DataFieldStartAddress+1] = 5;

	SavePara_StringType(32,&HartDataBuffer[DataFieldStartAddress+2]);
	for(TempC = FrameByteNuber; TempC >= (DataFieldStartAddress+2); TempC++)
	{
		HartDataBuffer[TempC+2] = HartDataBuffer[TempC];
	}
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;
	TxNumber = FrameByteNuber+2;
}

void	HARTCMD34(void)//WRITE PRIMARY VARIABLE DAMPING VALUE
{
	uint8_t TempC;
//	float TempF;
	TempC = (uint8_t)(RFrameCharToFloat(4));
	SavePara_CharType(11,TempC);
	HartDataBuffer[DataFieldStartAddress+1] = 6;

	for(TempC = FrameByteNuber; TempC >= (DataFieldStartAddress+2); TempC++)
	{
		HartDataBuffer[TempC+2] = HartDataBuffer[TempC];
	}
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;
	TxNumber = FrameByteNuber+2;
}

void	HARTCMD35(void)//WRITE PRIMARY VARIABLE DAMPING VALUE
{
	uint8_t TempC;
	float TempF[2];
	TempF[1] = RFrameCharToFloat(5);
	TempF[0] = RFrameCharToFloat(9);
	SavePara_FloatType(34,&TempF[0],2);
	HartDataBuffer[DataFieldStartAddress+1] = HartDataBuffer[DataFieldStartAddress+1]+2;

	for(TempC = FrameByteNuber; TempC >= (DataFieldStartAddress+2); TempC++)
	{
		HartDataBuffer[TempC+2] = HartDataBuffer[TempC];
	}
	HartDataBuffer[DataFieldStartAddress+2] = CommNormal;
	HartDataBuffer[DataFieldStartAddress+3] = DeviceNormal;
	TxNumber = FrameByteNuber+2;
}

void	AnalyzeHartProtocol(void)
{
//	uint8_t TempC,TempM;
	if ( RFlag == HART_RFRAME_TRUE )
	{
		RFlag = HART_RFRAME_FAIL;
		HartDataBuffer[0] |= 0x04;
		switch ( HartDataBuffer[DataFieldStartAddress] )
		{
			case 0:
				HARTCMD0();
				break;
			case 1:
				HARTCMD1();
				break;
			case 2:
				HARTCMD2();
				break;
			case 3:
				HARTCMD3();
				break;
			case 6:
				HARTCMD6();
				break;
			case 12:
				HARTCMD12();
				break;
			case 13:
				HARTCMD13();
				break;
			case 14:
				HARTCMD14();
				break;
			case 15:
				HARTCMD15();
				break;
			case 16:
				HARTCMD16();
				break;
			case 18:
				HARTCMD18();
				break;
			case 19:
				HARTCMD19();
				break;
			case 34:
				HARTCMD34();
				break;
			case 35:
				HARTCMD35();
				break;
/*			case 36:
				HARTCMD36();
				break;
			case 37:
				HARTCMD37();
				break;
			case 40:
				HARTCMD40();
				break;
			case 44:
				HARTCMD44();
				break;
			case 45:
				HARTCMD45();
				break;
			case 46:
				HARTCMD46();
				break;
			case 49:
				HARTCMD49();
				break;
			case 70:
				HARTCMD70();
				break;
			case 71:
				HARTCMD71();
				break;
			case 73:
				HARTCMD73();
				break;
			case 80:
				HARTCMD80();
				break;
			case 81:
				HARTCMD81();
				break;
			case 82:
				HARTCMD82();
				break;
			case 83:
				HARTCMD83();
				break;
			case 84:
				HARTCMD84();
				break;
			case 85:
				CMD_TEST = HARTCMD85();
				break;
			case 86:
				CMD_TEST = HARTCMD86();
				break;
			case 87:
				CMD_TEST = HARTCMD87();
				break;
			case 88:
				CMD_TEST = HARTCMD88();
				break;
			case 89:
				CMD_TEST = HARTCMD89();
				break;
			case 90:
				CMD_TEST = HARTCMD90();
				break;
			case 149:
				CMD_TEST = HARTCMD149();
				break;
			case 150:
				CMD_TEST = HARTCMD150();
				break;
			case 151:
				CMD_TEST = HARTCMD151();
				break;
			case 152:
//				CMD_TEST = HARTCMD152();
				break;
			case 153:
				CMD_TEST = HARTCMD153();
				break;
			case 154:
				CMD_TEST = HARTCMD154();
				break;
			case 155:
				CMD_TEST = HARTCMD155();
				break;
			case 156:
				HARTCMD156();
				break;
			case 157:
				HARTCMD157();
				break;
			case 160:
				CMD_TEST = HARTCMD160();
				break;
			case 161:
				CMD_TEST = HARTCMD161();
				break;
			case 162:
				CMD_TEST = HARTCMD162();
				break;
			case 163:
				CMD_TEST = HARTCMD163();
				break;
			case 164:
				CMD_TEST = HARTCMD164();
				break;
			case 174:
				CMD_TEST = HARTCMD174();
				break;
			case 186:
				CMD_TEST = HARTCMD186();
				break;
			case 187:
				CMD_TEST = HARTCMD187();
				break;
			case 188:
				HARTCMD188();
				break;
			case 189:
				HARTCMD189();
				break;
			case 190:
			case 191:
			case 192:
			case 193:
			case 194:
				TempM = CMD-190;	
				TempM = 20*TempM;
				Hart_TFrame.DataLenth=20;
				HartDataBuffer[] = CommNormal;
				Hart_TFrame.StatusL = DeviceNormal;

				for(TempC = 0; TempC < 20; TempC++)
					Hart_TFrame.rdata[TempC] = LcdEcho[TempM+TempC];
				if(CMD == 194)
					UpdateCurveFlag = 1;
				break;
			
			case 216:
			 	CMD_TEST = HARTCMD216();//WRITE PV_UPPER_SENSOR_LIMIT
				break;
			case 230:
				CMD_TEST = HARTCMD230();
				break;
			case 231:
				CMD_TEST = HARTCMD231();
				break;
			case 234:
//				CMD_TEST = HARTCMD234();
				break;
			case 235:
//				CMD_TEST = HARTCMD235();
				break;
			case 236:
//				CMD_TEST = HARTCMD236();
				break;
			case 237:
//				CMD_TEST = HARTCMD237();
				break;
			case 240:
				CMD_TEST = HARTCMD240();
				break;
			case 241:
				CMD_TEST = HARTCMD241();
				break;
			case 242:
				CMD_TEST = HARTCMD242();
				break;
			case 243:
				CMD_TEST = HARTCMD243();
				break;
			case 244:
				CMD_TEST = HARTCMD244();
				break;
			case 245:
				CMD_TEST = HARTCMD245();
				break;
			case 246:
				CMD_TEST = HARTCMD246();
				break;
			case 249:
				CMD_TEST = HARTCMD249();
				break;
			case 250:
				CMD_TEST = HARTCMD250();
				break;
			case 251:
				CMD_TEST = HARTCMD251();
				break;
			case 252:
				CMD_TEST = HARTCMD252();
				break;
			case 253:
				CMD_TEST = HARTCMD253();
				break;
			case 254:
				CMD_TEST = HARTCMD254();
				break;
*/			default:
				break;
		}
		EnableHartTx();
	}	
}

static HAL_StatusTypeDef Hart_Receive_IT(void)
{
//	uint16_t* tmp;
	uint8_t  TempC,TempAddress;
	uint16_t  uhdata;

	/* Check that a Rx process is ongoing */
	if(huart1.RxState == HAL_UART_STATE_BUSY_RX)
	{
		uhdata = (uint16_t) READ_REG(huart1.Instance->RDR)&0x00FF;
		
		if((uhdata == 0xFF)&&(DelimiterFlag != 0x5A))
		{
			DelimiterFlag++;
		}
		else
		{
			if(DelimiterFlag >= DEFAULT_PREAM)
			{
				if(DelimiterFlag != 0x5A)
				{
					DelimiterFlag = 0x5A;
				}
				HartDataBuffer[TxRxCount] = uhdata;
				if(HartDataBuffer[0]== SHORTFRAME_M)
				{
					DataFieldStartAddress = 2;
					if(TxRxCount == 3)
						FrameByteNuber = TxRxCount+HartDataBuffer[3]+1;
				}
				else if(HartDataBuffer[0]== LONGFRAME_M)
				{
					DataFieldStartAddress = 6;
					if(TxRxCount == 7)
						FrameByteNuber = TxRxCount+HartDataBuffer[7]+1;
				}
				TxRxCount++;
				if(TxRxCount > FrameByteNuber)
				{
					/* Disable the UART Parity Error Interrupt and RXNE interrupt*/
					CLEAR_BIT(huart1.Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
					/* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
					CLEAR_BIT(huart1.Instance->CR3, USART_CR3_EIE);
					/* Rx process is completed, restore huart->RxState to Ready */
					huart1.RxState = HAL_UART_STATE_READY;
					HAL_UART_RxCpltCallback(&huart1);
					
					if(ReckonHartCRC(TxRxCount-1) == HartDataBuffer[TxRxCount-1])
					{
						TxRxCount = 0;
						if(GetComMethod() == 0)
							RFlag = HART_RFRAME_TRUE;
//							AnalyzeHartProtocol();
						else
						{
							if((HartDataBuffer[0] == SHORTFRAME_M)&&((HartDataBuffer[1]&0x0F) == GetComAddress()))
							{
//								AnalyzeHartProtocol();
								RFlag = HART_RFRAME_TRUE;
							}
							else if(HartDataBuffer[0] == LONGFRAME_M)
							{
								TempC = HartDataBuffer[1]&0x3F;
								TempAddress = DEVICE_ID2+GetComAddress();
								if((TempC == MANUFACTURE_ID)&&(HartDataBuffer[2] == DEVICE_TYPE)&&(HartDataBuffer[3] == DEVICE_ID0)&&(HartDataBuffer[4] == DEVICE_ID1)&&(HartDataBuffer[5] == TempAddress))
								{
//									AnalyzeHartProtocol();
									RFlag = HART_RFRAME_TRUE;
								}
							}
						}
					}
					return HAL_OK;
				}
			}
		}
		return HAL_OK;
	}
	else
	{
		/* Clear RXNE interrupt flag */
		__HAL_UART_SEND_REQ(&huart1, UART_RXDATA_FLUSH_REQUEST);
		return HAL_BUSY;
	}
}

static HAL_StatusTypeDef Hart_Transmit_IT()
{
//	uint16_t* tmp;
	/* Check that a Tx process is ongoing */
	if (huart1.gState == HAL_UART_STATE_BUSY_TX)
	{
		if(TxRxCount > TxNumber)
		{
			/* Disable the UART Transmit Data Register Empty Interrupt */
			CLEAR_BIT(huart1.Instance->CR1, USART_CR1_TXEIE);
			/* Enable the UART Transmit Complete Interrupt */
			SET_BIT(huart1.Instance->CR1, USART_CR1_TCIE);
			return HAL_OK;
		}
		else
		{
			huart1.Instance->TDR = (uint8_t)(HartDataBuffer[TxRxCount]);
			TxRxCount++;
			return HAL_OK;
		}
	}
	else
	{
		return HAL_BUSY;
	}
}

void	UART1_IRQHandler(void)
{
	uint32_t isrflags   = READ_REG(huart1.Instance->ISR);
	uint32_t cr1its     = READ_REG(huart1.Instance->CR1);
	uint32_t cr3its;
	uint32_t errorflags;

	/* If no error occurs */
	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
	if (errorflags == RESET)
	{
		/* UART in mode Receiver ---------------------------------------------------*/
		if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{
			Hart_Receive_IT();
			return;
		}
	}  

	/* If some errors occur */
	cr3its = READ_REG(huart1.Instance->CR3);
	if( (errorflags != RESET)&& ( ((cr3its & USART_CR3_EIE) != RESET)|| ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)) )
	{
		/* UART parity error interrupt occurred -------------------------------------*/
		if(((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_PEF);
			huart1.ErrorCode |= HAL_UART_ERROR_PE;
		}

		/* UART frame error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_FEF);

			huart1.ErrorCode |= HAL_UART_ERROR_FE;
		}

		/* UART noise error interrupt occurred --------------------------------------*/
		if(((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
		{
			__HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_NEF);
			huart1.ErrorCode |= HAL_UART_ERROR_NE;
		}
    
		/* UART Over-Run interrupt occurred -----------------------------------------*/
		if(((isrflags & USART_ISR_ORE) != RESET)&&(((cr1its & USART_CR1_RXNEIE) != RESET) || ((cr3its & USART_CR3_EIE) != RESET)))
		{
			__HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_OREF);
			huart1.ErrorCode |= HAL_UART_ERROR_ORE;
		}

		/* Call UART Error Call back function if need be --------------------------*/
		if(huart1.ErrorCode != HAL_UART_ERROR_NONE)
		{
			/* UART in mode Receiver ---------------------------------------------------*/
			if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
			{
				Hart_Receive_IT();
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
		huart1.gState  = HAL_UART_STATE_READY;
		huart1.RxState = HAL_UART_STATE_READY;
		HAL_UARTEx_WakeupCallback(&huart1);
		return;
	}

	/* UART in mode Transmitter ------------------------------------------------*/
	if(((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
	{
		Hart_Transmit_IT();
		return;
	}

	/* UART in mode Transmitter (transmission end) -----------------------------*/
	if(((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
//		UART_EndTransmit_IT(&huart1);
		/* Disable the UART Transmit Complete Interrupt */
		CLEAR_BIT(huart1.Instance->CR1, USART_CR1_TCIE);
		/* Tx process is ended, restore huart->gState to Ready */
		huart1.gState = HAL_UART_STATE_READY;
		HAL_UART_TxCpltCallback(&huart1);
		EnableHartRx();
		return;
	}

}
