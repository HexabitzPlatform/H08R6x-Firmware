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
}VL53L8CX_Status;



/* Exported functions  ---------------------------------------------*/
VL53L8CX_Status VL53L8CX_SetResolution(uint8_t res);

VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t freq);

// VL53L8CX_Status VL53L8CX_SetTargetsPerZone(uint8_t count);

VL53L8CX_Status VL53L8CX_SetRangingMode(uint8_t rangMode);

VL53L8CX_Status VL53L8CX_SetPowerMode(uint8_t pwrMode);

VL53L8CX_Status VL53L8CX_SampleRanging(void);



#endif /* VL53L8CX_APIs */
