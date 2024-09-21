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


StatusModule ModuleInit(void);

StatusModule SetResolution(uint8_t res);

StatusModule SetPwrMode(uint8_t pwr);

StatusModule SetRangingMode(uint8_t rang);


#endif /* INC_MODULE_H_ */
