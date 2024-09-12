/*
 * LSM303AGR_APIs.C
 * Description: LSM6DS3TR-C Accelerometer and magnetometer unit APIs driver header file.
 *  Created on: Jul 24, 2024
 *      Author: Adel Faki @ Hexabitz
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 Hexabitz.
 * All rights reserved.
 *
 ******************************************************************************
 */

#include "VL53L8CX_APIS.h"
#include "main.h"

/* Exported Type's instance  ---------------------------------------------*/
int status;
volatile int IntCount;
uint8_t p_data_ready;
VL53L8CX_Configuration 	Dev;
VL53L8CX_ResultsData Results;
uint8_t resolution, isAlive;
uint16_t idx;


void get_data_by_polling(VL53L8CX_Configuration *p_dev);

/*
 * ************** Local function *************************
 */
void get_data_by_polling(VL53L8CX_Configuration *p_dev){
	do
	{
		status = vl53l8cx_check_data_ready(&Dev, &p_data_ready);

		if(p_data_ready){
			status = vl53l8cx_get_resolution(p_dev, &resolution);
			status = vl53l8cx_get_ranging_data(p_dev, &Results);

			for(int i = 0; i < resolution;i++){
			//	 Print per zone results
				printf("Zone : %2d, Nb targets : %2u, Ambient : %4lu Kcps/spads, ",
						i,
						Results.nb_target_detected[i],
						Results.ambient_per_spad[i]);

			//	 Print per target results
				if(Results.nb_target_detected[i] > 0){
					printf("Target status : %3u, Distance : %4d mm\n",
							Results.target_status[VL53L8CX_NB_TARGET_PER_ZONE * i],
							Results.distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * i]);
				}else{
					printf("Target status : 255, Distance : No target\n");
				}
			}
			printf("\n");
			break;
		}else{
			HAL_Delay(5);
		}
	}
	while(1);

}

/**************************************************************************/
/* Exported functions  ****************************************************/
/**************************************************************************/

VL53L8CX_Status VL53L8CX_Init(void){

	VL53L8CX_Reset_Sensor(&(Dev.platform));

	if(vl53l8cx_is_alive(&Dev, &isAlive))
		return VL53L8CX_ERR_INIT;
	if(!isAlive)
		return VL53L8CX_ERR_INIT;
	if(vl53l8cx_init(&Dev))
		return VL53L8CX_ERR_INIT;

	return VL53L8CX_OK;

}

VL53L8CX_Status VL53L8CX_SetResolution(uint8_t res){

	if(vl53l8cx_set_resolution(&Dev, res))
		return VL53L8CX_ERR_RES;
	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t freq){

	if(vl53l8cx_set_ranging_frequency_hz(&Dev, freq))				// Set 5Hz ranging frequency
		return VL53L8CX_ERR_Freq;
	return VL53L8CX_OK;
}

/*VL53L8CX_Status VL53L8CX_SetTargetsPerZone(uint8_t count){
	VL53L8CX_NB_TARGET_PER_ZONE = count;
	return VL53L8CX_OK;
}*/

VL53L8CX_Status VL53L8CX_SetRangingMode(uint8_t rangMode){
	vl53l8cx_set_ranging_mode(&Dev, rangMode);
	return VL53L8CX_OK;

}

VL53L8CX_Status VL53L8CX_SetPowerMode(uint8_t pwrMode){
	vl53l8cx_set_power_mode(&Dev, pwrMode);
		return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_SampleRanging(void){

	if(vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	get_data_by_polling(&Dev);
	return VL53L8CX_OK;
}
