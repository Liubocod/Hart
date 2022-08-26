/**
  ******************************************************************************
  * File Name          : AES.c
  * Description        : This file provides code for the configuration
  *                      of the AES instances.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2020 STMicroelectronics
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
#include "aes.h"

#include "dma.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CRYP_HandleTypeDef hcryp;
DMA_HandleTypeDef hdma_aes_in;

/* AES init function */
void MX_AES_Init(void)
{
   uint8_t pKey[16] ;

  hcryp.Instance = AES;
  hcryp.Init.DataType = CRYP_DATATYPE_32B;
  hcryp.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp.Init.OperatingMode = CRYP_ALGOMODE_ENCRYPT;
  hcryp.Init.ChainingMode = CRYP_CHAINMODE_AES_ECB;
  hcryp.Init.KeyWriteFlag = CRYP_KEY_WRITE_ENABLE;
  pKey[0] = 0x00;
  pKey[1] = 0x00;
  pKey[2] = 0x00;
  pKey[3] = 0x00;
  pKey[4] = 0x00;
  pKey[5] = 0x00;
  pKey[6] = 0x00;
  pKey[7] = 0x00;
  pKey[8] = 0x00;
  pKey[9] = 0x00;
  pKey[10] = 0x00;
  pKey[11] = 0x00;
  pKey[12] = 0x00;
  pKey[13] = 0x00;
  pKey[14] = 0x00;
  pKey[15] = 0x00;
  hcryp.Init.pKey = &pKey[0];
  if (HAL_CRYP_Init(&hcryp) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_CRYP_MspInit(CRYP_HandleTypeDef* crypHandle)
{

  if(crypHandle->Instance==AES)
  {
  /* USER CODE BEGIN AES_MspInit 0 */

  /* USER CODE END AES_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_AES_CLK_ENABLE();
  
    /* AES DMA Init */
    /* AES_IN Init */
    hdma_aes_in.Instance = DMA2_Channel5;
    hdma_aes_in.Init.Request = DMA_REQUEST_6;
    hdma_aes_in.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_aes_in.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_aes_in.Init.MemInc = DMA_MINC_ENABLE;
    hdma_aes_in.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_aes_in.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_aes_in.Init.Mode = DMA_NORMAL;
    hdma_aes_in.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_aes_in) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(crypHandle,hdmain,hdma_aes_in);

  /* USER CODE BEGIN AES_MspInit 1 */

  /* USER CODE END AES_MspInit 1 */
  }
}

void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef* crypHandle)
{

  if(crypHandle->Instance==AES)
  {
  /* USER CODE BEGIN AES_MspDeInit 0 */

  /* USER CODE END AES_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_AES_CLK_DISABLE();

    /* AES DMA DeInit */
    HAL_DMA_DeInit(crypHandle->hdmain);
  /* USER CODE BEGIN AES_MspDeInit 1 */

  /* USER CODE END AES_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
