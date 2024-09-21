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

/* Exported macros -----------------------------------------------------------*/


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


#endif /* INC_MODULE_H_ */
