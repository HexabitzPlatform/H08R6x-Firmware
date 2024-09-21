/*
 File Name     : Module.c
 Description   : Source code for module Module.
 	 	 	 	 (Description_of_module)

(Description of Special module peripheral configuration):
>>
>>
>>

 */

/* Includes ------------------------------------------------------------------*/

#include "Module.h"



StatusModule ModuleInit(void){

	if(VL53L8CX_Init())
		return MODULE_ERR_INIT;

	if(VL53L8CX_SetResolution(VL53L8CX_APIs_RESOLUTIN_4))
		return MODULE_ERR_RES;

	if(VL53L8CX_SetFrequancy(VL53L8CX_APIs_FREQUANCY))
		return MODULE_ERR_Freq;

	if(VL53L8CX_SetRangingMode(VL53L8CX_APIs_RANGING_MODE_AUTONOMOUS))
		return MODULE_ERR_Rang;

	if(VL53L8CX_SetPowerMode(VL53L8CX_APIs_PWR_MODE_WAKEUP))
		return MODULE_ERR_PWR;

	return MODULE_OK;

}

StatusModule SetResolution(uint8_t res){

	if(res == RES_4_BY_4)
	{
		if(VL53L8CX_SetResolution(VL53L8CX_APIs_RESOLUTIN_4))
				return MODULE_ERR_RES;
	}
	else if(res == RES_8_BY_8)
	{
		if(VL53L8CX_SetResolution(VL53L8CX_APIs_RESOLUTIN_8))
				return MODULE_ERR_RES;
	}
	return MODULE_OK;

}

StatusModule SetPwrMode(uint8_t pwr){

	if(pwr == WAKEUP)
	{
		if(VL53L8CX_SetPowerMode(VL53L8CX_APIs_PWR_MODE_WAKEUP))
				return MODULE_ERR_PWR;
	}
	else if(pwr == SLEEP)
	{
		if(VL53L8CX_SetPowerMode(VL53L8CX_APIs_PWR_MODE_SLEEP))
				return MODULE_ERR_PWR;
	}
	else if(pwr == DEEP_SLEEP)
		{
			if(VL53L8CX_SetPowerMode(VL53L8CX_APIs_PWR_MODE_DEEP_SLEEP))
					return MODULE_ERR_PWR;
		}
	return MODULE_OK;

}
