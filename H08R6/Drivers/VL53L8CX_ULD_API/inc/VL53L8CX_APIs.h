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
#include "main.h"

/* Private macros ------------------------------------------------------------*/
#define VL53L8CX_APIs_RESOLUTIN_4	((uint8_t) 16U)
#define VL53L8CX_APIs_RESOLUTIN_8	((uint8_t) 64U)
#define VL53L8CX_APIs_FREQUANCY		5U
#define VL53L8CX_APIs_RANGING_MODE_AUTONOMOUS		((uint8_t) 3U)
#define VL53L8CX_APIs_RANGING_MODE_CONTINUOUS		((uint8_t) 1U)
#define VL53L8CX_APIs_PWR_MODE_SLEEP		((uint8_t) 0U)
#define VL53L8CX_APIs_PWR_MODE_WAKEUP		((uint8_t) 1U)
#define VL53L8CX_APIs_PWR_MODE_DEEP_SLEEP	((uint8_t) 2U)
#define VL53L8CX_APIs_SHARPENER	((uint8_t) 6U)   // Default value sharpener is 5%

/* Exported types ------------------------------------------------------------*/
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
	VL53L8CX_ERR_SHARPENER
}VL53L8CX_Status;

typedef struct
{
	/* Internal sensor silicon temperature */
	int8_t silicon_temp_degc;
	/* Ambient noise in kcps/spads */
	uint32_t ambient_per_spad[VL53L8CX_RESOLUTION_8X8];
	/* Number of valid target detected for 1 zone */
	uint8_t nb_target_detected[VL53L8CX_RESOLUTION_8X8];
	/* Number of spads enabled for this ranging */
	uint32_t nb_spads_enabled[VL53L8CX_RESOLUTION_8X8];
	/* Signal returned to the sensor in kcps/spads */
	uint32_t signal_per_spad[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
	/* Sigma of the current distance in mm */
	uint16_t range_sigma_mm[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
	/* Measured distance in mm */
	int16_t distance_mm[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
	/* Estimated reflectance in percent */
	uint8_t reflectance[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
	/* Status indicating the measurement validity (5 & 9 means ranging OK)*/
	uint8_t target_status[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
	/* Motion detector results */
	struct
	{
		uint32_t global_indicator_1;
		uint32_t global_indicator_2;
		uint8_t	 status;
		uint8_t	 nb_of_detected_aggregates;
		uint8_t	 nb_of_aggregates;
		uint8_t	 spare;
		uint32_t motion[32];
	} motion_indicator;
} VL53L8CX_APIs_ResultsData;

/* Exported functions  ---------------------------------------------*/

VL53L8CX_Status VL53L8CX_Init(void);

VL53L8CX_Status VL53L8CX_SetResolution(uint8_t res);

VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t freq);

VL53L8CX_Status VL53L8CX_SetRangingMode(uint8_t rangMode);

VL53L8CX_Status VL53L8CX_SetPowerMode(uint8_t pwrMode);

VL53L8CX_Status VL53L8CX_SampleRanging(VL53L8CX_APIs_ResultsData* data);

VL53L8CX_Status VL53L8CX_StopRanging(void);

VL53L8CX_Status VL53L8CX_GetIntegrationTime(uint32_t* integration_time_ms);

VL53L8CX_Status VL53L8CX_SetIntegrationTime(uint32_t integration_time_ms);

VL53L8CX_Status VL53L8CX_SetSharpener(uint8_t sharpener);

VL53L8CX_Status VL53L8CX_GetSharpener(uint8_t* sharpener);

VL53L8CX_Status VL53L8CX_Calibration();

VL53L8CX_Status VL53L8CX_GetCalibrationData(uint8_t* pDataCalibrate);

VL53L8CX_Status VL53L8CX_SetCalibrationData(uint8_t* pDataCalibrate);

VL53L8CX_Status VL53L8CX_Detection_Thresholds(VL53L8CX_APIs_ResultsData* data);

VL53L8CX_Status VL53L8CX_MotionIndicator(VL53L8CX_APIs_ResultsData* data);

VL53L8CX_Status VL53L8CX_VisualizeXtalk(void);

VL53L8CX_Status VL53L8CX_SYNCRanging(VL53L8CX_APIs_ResultsData* data);

VL53L8CX_Status VL53L8CX_GetResolution(uint8_t *res);

#endif /* VL53L8CX_APIs */
/************************ (C) COPYRIGHT Hexabitz *****END OF FILE****/
