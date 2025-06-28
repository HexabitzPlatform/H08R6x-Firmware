/*
 BitzOS (BOS) V0.3.6 - Copyright (C) 2017-2024 Hexabitz
 All rights reserved
 
 File Name     : H08R6.h
 Description   : Header file for module H08R6.
 	 	 	 	 (Description_of_module)

(Description of Special module peripheral configuration):
>>
>>
>>

 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef H08R6_H
#define H08R6_H

/* Includes ------------------------------------------------------------------*/
#include "BOS.h"

#include "H08R6_MemoryMap.h"
#include "H08R6_uart.h"
#include "H08R6_gpio.h"
#include "H08R6_spi.h"
#include "H08R6_dma.h"
#include "H08R6_inputs.h"
#include "H08R6_eeprom.h"



/* Exported definitions -------------------------------------------------------*/

#define	MODULE_PN		_H08R6

#define	NUM_OF_PORTS	    6

#define P_PROG 				P2						/* ST factory bootloader UART */

/* Define available ports */
#define _P1
#define _P2
#define _P3
#define _P4
#define _P5
#define _P6

/* Define Available USARTs */
#define _USART1
#define _USART2
#define _USART3
#define _USART4
#define _USART5
#define _USART6

/* Port-UART mapping */
#define UART_P1 &huart4
#define UART_P2 &huart2
#define UART_P3 &huart3
#define UART_P4 &huart1
#define UART_P5 &huart5
#define UART_P6 &huart6

/* Port Definitions */
#define	USART1_TX_PIN		GPIO_PIN_9
#define	USART1_RX_PIN		GPIO_PIN_10
#define	USART1_TX_PORT		GPIOA
#define	USART1_RX_PORT		GPIOA
#define	USART1_AF			GPIO_AF1_USART1

#define	USART2_TX_PIN		GPIO_PIN_2
#define	USART2_RX_PIN		GPIO_PIN_3
#define	USART2_TX_PORT		GPIOA
#define	USART2_RX_PORT		GPIOA
#define	USART2_AF			GPIO_AF1_USART2

#define	USART3_TX_PIN		GPIO_PIN_10
#define	USART3_RX_PIN		GPIO_PIN_11
#define	USART3_TX_PORT		GPIOB
#define	USART3_RX_PORT		GPIOB
#define	USART3_AF			GPIO_AF4_USART3

#define	USART4_TX_PIN		GPIO_PIN_0
#define	USART4_RX_PIN		GPIO_PIN_1
#define	USART4_TX_PORT		GPIOA
#define	USART4_RX_PORT		GPIOA
#define	USART4_AF			GPIO_AF4_USART4

#define	USART5_TX_PIN		GPIO_PIN_3
#define	USART5_RX_PIN		GPIO_PIN_2
#define	USART5_TX_PORT		GPIOD
#define	USART5_RX_PORT		GPIOD
#define	USART5_AF			GPIO_AF3_USART5

#define	USART6_TX_PIN		GPIO_PIN_8
#define	USART6_RX_PIN		GPIO_PIN_9
#define	USART6_TX_PORT		GPIOB
#define	USART6_RX_PORT		GPIOB
#define	USART6_AF			GPIO_AF3_USART6

/* Module-specific Definitions */

/* Indicator LED */
#define _IND_LED_PORT		GPIOB
#define _IND_LED_PIN		GPIO_PIN_1


#define MIN_MEMS_PERIOD_MS	100
#define MAX_MEMS_TIMEOUT_MS	0xFFFFFFFF
#define NUM_MODULE_PARAMS	1

#define MIN_PERIOD_MS		100
/* Module special parameters */
#define SAMPLE_TO_PORT          1
#define STREAM_TO_PORT          2
#define STREAM_TO_Terminal      3
#define DEFAULT                 4

/* Module EEPROM Variables */
// Module Addressing Space 500 - 599
#define _EE_MODULE			500

/* Exported types ------------------------------------------------------------*/


/* Module_Status Type Definition */
typedef enum {
	H08R6_OK =0,
	H08R6_ERR_WrongMode,
	H08R6_ERR_WrongParams,
	H08R6_ERR_TERMINATED,
	H08R6_ERR_UnknownMessage,
	H08R6_ERROR =255
} Module_Status;

/* Export Module typedef structure */
/* Choose the functionality of stream and sample APIs */
typedef enum {
	AVERAGE =0, ALL,
} All_Data;

/* Indicator LED */
#define _IND_LED_PORT			GPIOB
#define _IND_LED_PIN			GPIO_PIN_1

/* Export UART variables */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;

/* Define UART Init prototypes */
extern void MX_USART1_UART_Init(void);
extern void MX_USART2_UART_Init(void);
extern void MX_USART3_UART_Init(void);
extern void MX_USART4_UART_Init(void);
extern void MX_USART5_UART_Init(void);
extern void MX_USART6_UART_Init(void);
extern void SystemClock_Config(void);
extern void ExecuteMonitor(void);



/* -----------------------------------------------------------------------
 |								  APIs							          |  																 	|
/* -----------------------------------------------------------------------
 */
Module_Status SampleDistanceAverage(int16_t *average);
Module_Status SampletoPort(uint8_t module,uint8_t port,All_Data function);
Module_Status StreamtoPort(uint8_t module,uint8_t port,All_Data function,uint32_t Numofsamples,uint32_t timeout);
Module_Status StreamToTerminal(uint8_t port,All_Data function,uint32_t Numofsamples,uint32_t timeout);
Module_Status StreamToBuffer(int16_t *buffer,All_Data function, uint32_t Numofsamples, uint32_t timeout);


void SetupPortForRemoteBootloaderUpdate(uint8_t port);
void remoteBootloaderUpdate(uint8_t src,uint8_t dst,uint8_t inport,uint8_t outport);

/* -----------------------------------------------------------------------
 |								Commands							      |															 	|
/* -----------------------------------------------------------------------
 */


#endif /* H08R6_H */

/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
