/**
  ******************************************************************************
  * File Name          : CRC.c
  * Description        : This file provides code for the configuration
  *                      of the CRC instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "crc.h"

/* USER CODE BEGIN 0 */
#define CRC_POLYNOMIAL_16B 			0x1021 /* X^16 + X^12 + X^5 + 1 */

/* USER CODE END 0 */
CRC_HandleTypeDef hcrc;

/* CRC init function */
void MX_CRC_Init(void)
{

  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  hcrc.Init.GeneratingPolynomial = CRC_POLYNOMIAL_16B;
  hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  hcrc.Instance->INIT = 0;
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspInit 0 */

  /* USER CODE END CRC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CRC_CLK_ENABLE();
  /* USER CODE BEGIN CRC_MspInit 1 */

  /* USER CODE END CRC_MspInit 1 */
  }
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* crcHandle)
{

  if(crcHandle->Instance==CRC)
  {
  /* USER CODE BEGIN CRC_MspDeInit 0 */

  /* USER CODE END CRC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CRC_CLK_DISABLE();
  /* USER CODE BEGIN CRC_MspDeInit 1 */

  /* USER CODE END CRC_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
uint16_t CRC_Accumulate(uint8_t pBuffer[], uint16_t BufferLength)
{
	uint16_t i = 0; /* input data buffer index */
	hcrc.Instance = CRC;
	uint16_t temp = 0;  /* CRC output (read from hcrc->Instance->DR register) */
	__HAL_LOCK(&hcrc); 
	hcrc.State = HAL_CRC_STATE_BUSY;
	__HAL_CRC_DR_RESET(&hcrc);
	/* Change CRC peripheral state */  
	hcrc.State = HAL_CRC_STATE_BUSY;

	for(i = 0; i < (BufferLength/4); i++)
	{
		hcrc.Instance->DR = ((uint32_t)pBuffer[4*i]<<24) | ((uint32_t)pBuffer[4*i+1]<<16) | ((uint32_t)pBuffer[4*i+2]<<8) | (uint32_t)pBuffer[4*i+3];      
	}
	/* last bytes specific handling */
	temp = BufferLength%4;
	switch(temp)
	{
		case 1:
			 *(uint8_t volatile*) (&hcrc.Instance->DR) = pBuffer[4*i];
			break;
		case 2:
			*(uint16_t volatile*) (&hcrc.Instance->DR) = ((uint32_t)pBuffer[4*i]<<8) | (uint32_t)pBuffer[4*i+1];
			break;
		case 3:
			*(uint16_t volatile*) (&hcrc.Instance->DR) = ((uint32_t)pBuffer[4*i]<<8) | (uint32_t)pBuffer[4*i+1] ;
			*(uint8_t volatile*) (&hcrc.Instance->DR) = pBuffer[4*i+2];
		break;
		default:break;
	}
  
	temp = hcrc.Instance->DR;
	/* Change CRC peripheral state */    
	hcrc.State = HAL_CRC_STATE_READY; 
  
	/* Process unlocked */
	__HAL_UNLOCK(&hcrc);
  
	/* Return the CRC computed value */ 
	return (temp);
}


/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
