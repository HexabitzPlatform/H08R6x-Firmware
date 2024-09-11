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

/* Exported types ------------------------------------------------------------*/
typedef enum {
	VL53L8CX_OK = 0,
	VL53L8CX_ERR = 100,
}VL53L8CX_Status;


/* Exported functions  ---------------------------------------------*/
VL53L8CX_Status VL53L8CX_Init(void);

VL53L8CX_Status VL53L8CX_SetResolution(void);

VL53L8CX_Status VL53L8CX_SampleRanging(void);



#endif /* VL53L8CX_APIs */
