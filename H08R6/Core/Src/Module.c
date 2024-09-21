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

StatusModule SetRangingMode(uint8_t rang){
	if(rang == AUTONOMOUS)
	{
		if(VL53L8CX_SetRangingMode(VL53L8CX_APIs_RANGING_MODE_AUTONOMOUS))
				return MODULE_ERR_Rang;
	}
	else if(rang == CONTINUOUS)
	{
		if(VL53L8CX_SetRangingMode(VL53L8CX_APIs_RANGING_MODE_CONTINUOUS))
				return MODULE_ERR_Rang;
	}

	return MODULE_OK;

}

StatusModule SampleRangingAllData(ModuleResults* m_results){
	VL53L8CX_APIs_ResultsData mData;
	uint8_t mRes;

	if(VL53L8CX_SampleRanging(&mData))
		return MODULE_ERR_Rang;
	else
	{
		VL53L8CX_GetResolution(&mRes);
		for(int i = 0; i < mRes;i++)
		{
			m_results->distance_mm[i]=mData.distance_mm[i];
			m_results->range_sigma_mm[i]=mData.range_sigma_mm[i];
			m_results->reflectance[i]=mData.reflectance[i];
			m_results->target_status[i]=mData.target_status[i];
			m_results->nb_target_detected[i]=mData.nb_target_detected[i];
			m_results->signal_per_spad[i]=mData.signal_per_spad[i];
			m_results->ambient_per_spad[i]=mData.ambient_per_spad[i];
			m_results->nb_spads_enabled[i]=mData.nb_spads_enabled[i];
		}
		m_results->silicon_temp_degc=mData.silicon_temp_degc;

	}

	return MODULE_OK;

}


