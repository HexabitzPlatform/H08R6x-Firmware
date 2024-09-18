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
#include "vl53l8cx_plugin_xtalk.h"
#include "vl53l8cx_plugin_detection_thresholds.h"
#include "main.h"

/* Exported Type's instance  ---------------------------------------------*/
int status;
volatile int IntCount;
uint8_t p_data_ready;
VL53L8CX_Configuration 	Dev;
VL53L8CX_ResultsData Results;
uint8_t resolution, isAlive;
uint16_t idx;
/* In this example, we want 2 thresholds per zone for a 4x4 resolution */
		/* Create array of thresholds (size cannot be changed) */
VL53L8CX_DetectionThresholds thresholds[VL53L8CX_NB_THRESHOLDS];

#define is_interrupt 1 /*is_interrupt = 1 => get data by interrupt, = 0 => get data by polling */



void get_data_by_polling(VL53L8CX_Configuration *p_dev);
void get_data_by_interrupt(VL53L8CX_Configuration *p_dev , uint8_t data_to_transfer, VL53L8CX_APIs_ResultsData* data);
void get_data_throshold(VL53L8CX_Configuration *p_dev);

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin==INT_Pin)
	{
		IntCount++;
	}
}


/*
 * ************** Local function *************************
 */

void get_data_by_interrupt(VL53L8CX_Configuration *p_dev , uint8_t data_to_transfer, VL53L8CX_APIs_ResultsData* data){
	do
	{
		__WFI();	// Wait for interrupt
		if(IntCount !=0 ){
			IntCount=0;
			status = vl53l8cx_get_resolution(p_dev, &resolution);
			status = vl53l8cx_get_ranging_data(p_dev, &Results);
			for(int i = 0; i < resolution;i++)
			{
					data->distance_mm[i]=Results.distance_mm[i];
					data->range_sigma_mm[i]=Results.range_sigma_mm[i];
					data->reflectance[i]=Results.reflectance[i];
					data->target_status[i]=Results.target_status[i];
					data->nb_target_detected[i]=Results.nb_target_detected[i];
					data->signal_per_spad[i]=Results.signal_per_spad[i];
					data->ambient_per_spad[i]=Results.ambient_per_spad[i];
					data->nb_spads_enabled[i]=Results.nb_spads_enabled[i];
			}
			data->silicon_temp_degc=Results.silicon_temp_degc;

			for(int i = 0; i < resolution;i++){
				/* Print per zone results */
				printf("Zone : %2d, Nb targets : %2u, Ambient : %4lu Kcps/spads, ",
						i,
						Results.nb_target_detected[i],
						Results.ambient_per_spad[i]);

				/* Print per target results */
				if(Results.nb_target_detected[i] > 0){
					printf("Target status : %3u, Distance : %4d mm\n",
							Results.target_status[VL53L8CX_NB_TARGET_PER_ZONE * i],
							Results.distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * i]);
				}else{
					printf("Target status : 255, Distance : No target\n");
				}
			}
			printf("\n");
		}
	}while(1);
}

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


void get_data_throshold(VL53L8CX_Configuration *p_dev){
	do
	{
		__WFI();	// Wait for interrupt
				if(IntCount !=0 ){
					IntCount=0;

		  		// Get the sensor's ranging data
		  		status = vl53l8cx_get_ranging_data(p_dev, &Results);
		  		printf("n");
		  		// Loop to print data of all 16 regions
		  		for (int i = 0; i < 16; i++) {
		  			printf("Zone : %3d, Status : %3u, Distance : %4d mm, Signal : %5lu kcps/SPADs\r\n",
		  							 i,
									 Results.target_status[VL53L8CX_NB_TARGET_PER_ZONE * i],
									 Results.distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * i],
									 Results.signal_per_spad[VL53L8CX_NB_TARGET_PER_ZONE * i]);
		  		}
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

VL53L8CX_Status VL53L8CX_SampleRanging(uint8_t data_to_transfer, VL53L8CX_APIs_ResultsData* data){

	if(vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	if (is_interrupt) {
		get_data_by_interrupt(&Dev, data_to_transfer, data);
	}
	else {
		get_data_by_polling(&Dev);
	}
	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_Detection_Thresholds(void)
{


		/* Set all values to 0 */
		memset(&thresholds, 0, sizeof(thresholds));

		/* Add thresholds for all zones (16 zones in resolution 4x4, or 64 in 8x8) */
			for(int i = 0; i < 16; i++){
				/* The first wanted thresholds is GREATER_THAN mode. Please note that the
				 * first one must always be set with a mathematic_operation
				 * VL53L8CX_OPERATION_NONE.
				 * For this example, the signal thresholds is set to 150 kcps/spads
				 * (the format is automatically updated inside driver)
				 */
				thresholds[2*i].zone_num = i;
				thresholds[2*i].measurement = VL53L8CX_SIGNAL_PER_SPAD_KCPS;
				thresholds[2*i].type = VL53L8CX_GREATER_THAN_MAX_CHECKER;
				thresholds[2*i].mathematic_operation = VL53L8CX_OPERATION_NONE;
				thresholds[2*i].param_low_thresh = 1400;
				thresholds[2*i].param_high_thresh = 1500;

				/* The second wanted checker is IN_WINDOW mode. We will set a
				 * mathematical thresholds VL53L8CX_OPERATION_OR, to add the previous
				 * checker to this one.
				 * For this example, distance thresholds are set between 200mm and
				 * 400mm (the format is automatically updated inside driver).
				 */
				thresholds[2*i+1].zone_num = i;
				thresholds[2*i+1].measurement = VL53L8CX_DISTANCE_MM;
				thresholds[2*i+1].type = VL53L8CX_IN_WINDOW;
				thresholds[2*i+1].mathematic_operation = VL53L8CX_OPERATION_OR;
				thresholds[2*i+1].param_low_thresh = 200;
				thresholds[2*i+1].param_high_thresh = 400;
			}
			/* The last thresholds must be clearly indicated. As we have 32
			 * checkers (16 zones x 2), the last one is the 31 */
			thresholds[31].zone_num = VL53L8CX_LAST_THRESHOLD | thresholds[31].zone_num;

			/* Send array of thresholds to the sensor */
			vl53l8cx_set_detection_thresholds(&Dev, thresholds);

			/* Enable detection thresholds */
			vl53l8cx_set_detection_thresholds_enable(&Dev, 1);

			status = vl53l8cx_set_ranging_frequency_hz(&Dev, 10);

			IntCount = 0;
			status = vl53l8cx_start_ranging(&Dev);
			printf("Put an object between 200mm and 400mm to catch an interrupt\n");


			get_data_throshold(&Dev);

			return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_StopRanging(void){
	if(vl53l8cx_stop_ranging(&Dev))
			return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_SetSharpener(uint8_t sharpener){
	if(vl53l8cx_set_sharpener_percent(&Dev, sharpener))
		return VL53L8CX_ERR_SHARPENER;
	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_GetSharpener(uint8_t* sharpener){
	vl53l8cx_get_sharpener_percent(&Dev, sharpener);
	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_Calibration(){
	if(vl53l8cx_calibrate_xtalk(&Dev, 3, 2, 620))
		return VL53L8CX_ERR_CALIBRATE;
	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_GetCalibrationData(uint8_t* pDataCalibrate){
	if(vl53l8cx_get_caldata_xtalk(&Dev, pDataCalibrate))
		return VL53L8CX_ERR_CALIBRATE;
	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_SetCalibrationData(uint8_t* pDataCalibrate){
	if(vl53l8cx_get_caldata_xtalk(&Dev, pDataCalibrate))
			return VL53L8CX_ERR_CALIBRATE;
	return VL53L8CX_OK;
}


