/*
 * Porting.h
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

#ifndef PORTING_H_
#define PORTING_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "H08R6.h"
//#include "dma.h"
#include "H08R6_gpio.h"
#include "H08R6_spi.h"


/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum{
	STATUS_OK = 0,
	STATUS_INV,
	STATUS_TMOUT,

	STATUS_BUSY = 0x40,

	RGB_WRONGINTENSITY,
	RGB_WRONGCOLOR,
	RGB_LED_INIT_ERR,

	STC3117_WRONG_ADDRESS,
	STATUS_ERR=255
}Status_TypeDef;

/* GPIOs type define */
typedef GPIO_TypeDef GPIO_HANDLE;


/* SPI type define */
typedef SPI_HandleTypeDef SPI_HANDLE;


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define _DELAY_MS(TimeOut)       HAL_Delay(TimeOut)

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* Peripherals Drivers Functions */

/* GPIOs functions */
Status_TypeDef SetGPIOsPin(GPIO_HANDLE *GPIOx, uint16_t Pin);
Status_TypeDef ResetGPIOsPin(GPIO_HANDLE *GPIOx, uint16_t Pin);

/* SPI functions */
Status_TypeDef SendSPI(SPI_HANDLE *xPort, uint8_t pData[], uint16_t Size);
Status_TypeDef ReceiveSPI(SPI_HANDLE *xPort, uint8_t pData[], uint16_t Size);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#endif /* PORTING_H_ */

/************************ (C) COPYRIGHT Hexabitz *****END OF FILE****/
