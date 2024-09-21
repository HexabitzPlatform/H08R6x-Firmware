/*
 * Module.h
 *
 *  Created on: Sep 21, 2024
 *      Author: Control
 */

#ifndef INC_MODULE_H_
#define INC_MODULE_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "VL53L8CX_APIS.h"

/* Exported macros -----------------------------------------------------------*/
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
typedef enum
{
	MODULE_OK = 0,
	MODULE_ERR_INIT,
	MODULE_ERR_RES,
	MODULE_ERR_Freq,
	MODULE_ERR_Targ,
	MODULE_ERR_PWR,
	MODULE_ERR_Rang,
	MODULE_ERR_INTEG_TIME,
	MODULE_ERR_CALIBRATE,
	MODULE_ERR_SHARPENER
}StatusModule;

typedef enum
{
	WAKEUP = 0x00,
	SLEEP = 0x01,
	DEEP_SLEEP = 0x02,
}PwrMode;

typedef enum
{
	AUTONOMOUS = 0x00,
	CONTINUOUS = 0x01,
}RangingMode;

typedef enum
{
	RES_4_BY_4 = 0x00,
	RES_8_BY_8 = 0x01,
}Resolution;

typedef enum
{
	CLOSEST = 0x00,
	STRONGEST = 0x01,
}TargetOrder;

typedef enum
{
	ALL = 0x00,
	DISTANCE = 0x01,
	REFLECTANCE,
	STATUS,
}OutputResults;

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
} ModuleResults;


StatusModule ModuleInit(void);

StatusModule SetResolution(uint8_t res);

StatusModule SetPwrMode(uint8_t pwr);

StatusModule SetRangingMode(uint8_t rang);

StatusModule SampleRangingAllData(ModuleResults* m_results);


#endif /* INC_MODULE_H_ */
