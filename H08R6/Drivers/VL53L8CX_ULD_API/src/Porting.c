/*
 * Porting.c
 * Description: MomentPick Bracelet Porting header file
 *  Created on: Dec 12, 2023
 *      Author: Mohammad Alchehabi @ Hexabitz
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 Hexabitz.
 * All rights reserved.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "Porting.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/



/**************************************************************************/

void delay(uint32_t ms){

	HAL_Delay(ms);
}

/* APIs ----------------------------------------------------------------------*/


/* GPIOs functions */
/*
 * @brief :set a GPIO pin form a port high
 */
Status_TypeDef SetGPIOsPin(GPIO_HANDLE *GPIOx, uint16_t Pin)
{
	Status_TypeDef Status;

	if (NULL!=GPIOx)
	{
		HAL_GPIO_WritePin(GPIOx, Pin, GPIO_PIN_SET);
		Status=STATUS_OK;
	}
	else
		Status=STATUS_ERR;

	return Status;
}

/*
 * set a GPIO pin form a port low
 */
Status_TypeDef ResetGPIOsPin(GPIO_HANDLE *GPIOx, uint16_t Pin)
{
	Status_TypeDef Status;

	if (NULL!=GPIOx)
	{
		HAL_GPIO_WritePin(GPIOx, Pin, GPIO_PIN_RESET);
		Status=STATUS_OK;
	}
	else
		Status=STATUS_ERR;

	return Status;
}

/* SPI functions */
/*
 * @brief: send data buffer via SPI port
 * @param1: SPI port handle
 * @param2: Pointer to data buffer
 * @param3: data size in bytes unit
 * @retval: Status
 */
Status_TypeDef SendSPI(SPI_HANDLE *xPort, uint8_t pData[], uint16_t Size)
{
	Status_TypeDef Status=STATUS_ERR;

	if (NULL!=xPort && NULL!=pData)
	{
		if (HAL_OK == HAL_SPI_Transmit(xPort, pData, Size, 0x1000))  //TIM_OUT_1MS
			Status = STATUS_OK;
		}
	else
		Status = STATUS_ERR;

	return Status;
}

/*
 * @brief: receive data from SPI port and store in a buffer
 * @param1: SPI port handle
 * @param2: Pointer to data buffer
 * @param3: data size in bytes unit
 * @retval: Status
 */
Status_TypeDef ReceiveSPI(SPI_HANDLE *xPort, uint8_t pData[], uint16_t Size)
{
	Status_TypeDef Status;

	if (NULL!=xPort && NULL!=pData)
	{
		if (HAL_OK == HAL_SPI_Receive(xPort, pData, Size, 100*Size))  //TIM_OUT_1MS
			Status=STATUS_OK;
	}
	else
		Status=STATUS_ERR;

	return Status;
}

/************************ (C) COPYRIGHT Hexabitz *****END OF FILE****/
