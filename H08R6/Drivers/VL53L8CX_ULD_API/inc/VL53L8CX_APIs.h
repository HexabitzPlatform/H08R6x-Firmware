/*
 * VL53L8CX_APIs.h
 *
 *  Created on: Sep 10, 2024
 *      Author: Control
 */

#ifndef VL53L8CX_APIs
#define VL53L8CX_APIs

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "vl53l8cx_api.h"
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
	VL53L8CX_OK = 0,
	VL53L8CX_ERR_INIT,
	VL53L8CX_ERR_RES,
	VL53L8CX_ERR_Freq,
	VL53L8CX_ERR_Targ,
	VL53L8CX_ERR_PWR,
	VL53L8CX_ERR_Rang,
	VL53L8CX_ERR_CALIBRATE,
	VL53L8CX_ERR_SHARPENER
}VL53L8CX_Status;

typedef struct
{
	/* Internal sensor silicon temperature */
	int8_t silicon_temp_degc;

	/* Ambient noise in kcps/spads */
//#ifndef VL53L8CX_DISABLE_AMBIENT_PER_SPAD
	uint32_t ambient_per_spad[VL53L8CX_RESOLUTION_8X8];
// #endif

	/* Number of valid target detected for 1 zone */
// #ifndef VL53L8CX_DISABLE_NB_TARGET_DETECTED
	uint8_t nb_target_detected[VL53L8CX_RESOLUTION_8X8];
// #endif

	/* Number of spads enabled for this ranging */
// #ifndef VL53L8CX_DISABLE_NB_SPADS_ENABLED
	uint32_t nb_spads_enabled[VL53L8CX_RESOLUTION_8X8];
// #endif

	/* Signal returned to the sensor in kcps/spads */
// #ifndef VL53L8CX_DISABLE_SIGNAL_PER_SPAD
	uint32_t signal_per_spad[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
// #endif

	/* Sigma of the current distance in mm */
// #ifndef VL53L8CX_DISABLE_RANGE_SIGMA_MM
	uint16_t range_sigma_mm[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
// #endif

	/* Measured distance in mm */
// #ifndef VL53L8CX_DISABLE_DISTANCE_MM
	int16_t distance_mm[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
// #endif

	/* Estimated reflectance in percent */
// #ifndef VL53L8CX_DISABLE_REFLECTANCE_PERCENT
	uint8_t reflectance[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
// #endif

	/* Status indicating the measurement validity (5 & 9 means ranging OK)*/
// #ifndef VL53L8CX_DISABLE_TARGET_STATUS
	uint8_t target_status[(VL53L8CX_RESOLUTION_8X8
					*VL53L8CX_NB_TARGET_PER_ZONE)];
// #endif

	/* Motion detector results */
// #ifndef VL53L8CX_DISABLE_MOTION_INDICATOR
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
// #endif

} VL53L8CX_APIs_ResultsData;



/* Exported functions  ---------------------------------------------*/
VL53L8CX_Status VL53L8CX_SetResolution(uint8_t res);

VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t freq);

// VL53L8CX_Status VL53L8CX_SetTargetsPerZone(uint8_t count);

VL53L8CX_Status VL53L8CX_SetRangingMode(uint8_t rangMode);

VL53L8CX_Status VL53L8CX_SetPowerMode(uint8_t pwrMode);

VL53L8CX_Status VL53L8CX_SampleRanging(uint8_t data_to_transfer, VL53L8CX_APIs_ResultsData* data);

VL53L8CX_Status VL53L8CX_StopRanging(void);

VL53L8CX_Status VL53L8CX_SetSharpener(uint8_t sharpener);

VL53L8CX_Status VL53L8CX_GetSharpener(uint8_t* sharpener);

VL53L8CX_Status VL53L8CX_Calibration();

VL53L8CX_Status VL53L8CX_GetCalibrationData();

VL53L8CX_Status VL53L8CX_SetCalibrationData();

VL53L8CX_Status VL53L8CX_Detection_Thresholds(void);


#endif /* VL53L8CX_APIs */
