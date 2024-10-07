/*
 * VL53L8CX_APIS.h
 * Description: VL53L8CX TOF sensor APIs driver header file.
 *  Created on: Sep 6, 2024
 *      Author: Muhammad Alhaddad @ Hexabitz
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 Hexabitz.
 * All rights reserved.
 *
 ******************************************************************************
 */
#ifndef VL53L8CX_APIs
#define VL53L8CX_APIs

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "vl53l8cx_api.h"
#include "vl53l8cx_plugin_xtalk.h"
#include "vl53l8cx_plugin_detection_thresholds.h"
#include "vl53l8cx_plugin_motion_indicator.h"
//#include "BOS.h"
#include "Porting.h"

/* Private macros ------------------------------------------------------------*/
#define VL53L8CX_SPI_HANDLER    &hspi2

#define SPI_I2C_N_Pin 			GPIO_PIN_4
#define SPI_I2C_N_GPIO_Port 	GPIOA
#define SYNC_Pin 				GPIO_PIN_5
#define SYNC_GPIO_Port 			GPIOA
#define AVDD_EN_Pin 			GPIO_PIN_7
#define AVDD_EN_GPIO_Port 		GPIOA
#define CORE1_8_EN_Pin 			GPIO_PIN_0
#define CORE1_8_EN_GPIO_Port 	GPIOB
#define NCS_Pin 				GPIO_PIN_12
#define NCS_GPIO_Port 			GPIOB
#define INT_Pin 				GPIO_PIN_14
#define INT_GPIO_Port 			GPIOB
#define INT_EXTI_IRQn 			EXTI4_15_IRQn
#define LPn_Pin 				GPIO_PIN_8
#define LPn_GPIO_Port 			GPIOA


#define VL53L8CX_APIs_FREQUANCY		5U    /* 4x4 resolution max is 60 , 8x8 resolution max is 15 */
#define LOW_MOTION_INDICATOR	((uint16_t) 500U)
#define HIGH_MOTION_INDICATOR	((uint16_t) 1000U)

#define IS_INTERRUPT	 		0					/* 0 for polling mode and 1 for interrupt mode reading data*/

#define _DELAY_MS(TimeOut)      HAL_Delay(TimeOut)
#define _TIME_MS()				HAL_GetTick()

/* Exported types ------------------------------------------------------------*/
/* typedef enumeration Definitions */
typedef enum {
	VL53L8CX_OK = 0,
	VL53L8CX_ERR_INIT,
	VL53L8CX_ERR_RES,
	VL53L8CX_ERR_Freq,
	VL53L8CX_ERR_Targ,
	VL53L8CX_ERR_PWR,
	VL53L8CX_ERR_Rang,
	VL53L8CX_ERR_INTEG_TIME,
	VL53L8CX_ERR_CALIBRATE,
	VL53L8CX_ERR_SHARPENER,
	VL53L8CX_ERR_MOTION_IND
}VL53L8CX_Status;

typedef enum
{
	WAKEUP = 0x00,
	SLEEP = 0x01,
	DEEP_SLEEP = 0x02,
}PwrMode_e;

typedef enum
{
	AUTONOMOUS = 0x00,
	CONTINUOUS = 0x01,
}RangingMode_e;

typedef enum
{
	ZONES_4X4 = 0x00,
	ZONES_8X8 = 0x01,
}Resolution_e;

typedef struct
{
	uint32_t global_indicator_1;
	uint32_t global_indicator_2;
	uint8_t	 status;
	uint8_t	 nb_of_detected_aggregates;
	uint8_t	 nb_of_aggregates;
	uint8_t	 spare;
	uint32_t motion[32];
} VL53L8CX_APIs_motion_indicator;


/* User functions ---------------------------------------------*/
VL53L8CX_Status VL53L8CX_Init(void);
VL53L8CX_Status VL53L8CX_SetResolution(Resolution_e Res);
VL53L8CX_Status VL53L8CX_SetPowerMode(PwrMode_e Pwr);
VL53L8CX_Status VL53L8CX_SetRangingMode(RangingMode_e Rang);
VL53L8CX_Status VL53L8CX_xTalkCalibration(void);
VL53L8CX_Status VL53L8CX_SampleDistance(int16_t* Distance);
VL53L8CX_Status VL53L8CX_SampleDistanceAverage(int16_t* Distance_a);
VL53L8CX_Status VL53L8CX_StreamDistance(int16_t* Distance_a, uint32_t Time_out);
//VL53L8CX_Status VL53L8CX_NumberofTargets(int16_t* NofTargets);
//VL53L8CX_Status VL53L8CX_MotionIndicator(int16_t* Indicator);
//VL53L8CX_Status VL53L8CX_SampleRangingAllData(VL53L8CX_APIs_ResultsData* Data);

#endif /* VL53L8CX_APIs */
/************************ (C) COPYRIGHT Hexabitz *****END OF FILE****/
