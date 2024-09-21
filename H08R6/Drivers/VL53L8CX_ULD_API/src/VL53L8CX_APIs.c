/*
 * VL53L8CX_APIS.C
 * Description: VL53L8CX TOF sensor APIs driver source file.
 *  Created on: Sep 6, 2024
 *      Author: Muhammad Alhaddad @ Hexabitz
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 Hexabitz.
 * All rights reserved.
 *
 ******************************************************************************
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "VL53L8CX_APIS.h"


/* Exported Type's instance  ---------------------------------------------*/
int status;
volatile int IntCount;
uint8_t p_data_ready;
VL53L8CX_Configuration 	Dev;
VL53L8CX_ResultsData Results;
uint8_t resolution, isAlive;
VL53L8CX_Motion_Configuration 	motion_config;
uint16_t idx;
VL53L8CX_DetectionThresholds thresholds[VL53L8CX_NB_THRESHOLDS];
uint8_t					xtalk_data[VL53L8CX_XTALK_BUFFER_SIZE];
#define is_interrupt 1

/* Platform Exported Functions ********************************************/
void delay(uint32_t ms);

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin==INT_Pin)
	{
		IntCount++;
	}
}


/* Local functions prototypes ********************************************/
void get_data_by_polling(VL53L8CX_Configuration *p_dev, VL53L8CX_APIs_ResultsData* data);
void get_data_by_interrupt(VL53L8CX_Configuration *p_dev , VL53L8CX_APIs_ResultsData* data);
void get_data_throshold(VL53L8CX_Configuration *p_dev, VL53L8CX_APIs_ResultsData* data);
void get_motion_indicator(VL53L8CX_APIs_ResultsData* data);


/**************************************************************************/
/* Platform Exported Functions ********************************************/
/**************************************************************************/

void delay(uint32_t ms){

	HAL_Delay(ms);

}

/**************************************************************************/
/* Local Functions  *******************************************************/
/**************************************************************************/
void get_data_by_interrupt(VL53L8CX_Configuration *p_dev, VL53L8CX_APIs_ResultsData* data){
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
						data->nb_target_detected[i],
						data->ambient_per_spad[i]);

				/* Print per target results */
				if(data->nb_target_detected[i] > 0){
					printf("Target status : %3u, Distance : %4d mm\n",
							data->target_status[VL53L8CX_NB_TARGET_PER_ZONE * i],
							data->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * i]);
				}else{
					printf("Target status : 255, Distance : No target\n");
				}
			}
			printf("\n");
		}
	}while(1);
}

/**********************************************************************/

void get_data_by_polling(VL53L8CX_Configuration *p_dev, VL53L8CX_APIs_ResultsData* data){
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
						data->nb_target_detected[i],
						data->ambient_per_spad[i]);

			//	 Print per target results
				if(data->nb_target_detected[i] > 0){
					printf("Target status : %3u, Distance : %4d mm\n",
							data->target_status[VL53L8CX_NB_TARGET_PER_ZONE * i],
							data->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * i]);
				}else{
					printf("Target status : 255, Distance : No target\n");
				}
			}
			printf("\n");
			break;
		}else{
			delay(5);
		}
	}
	while(1);

}

/**********************************************************************/

void get_data_throshold(VL53L8CX_Configuration *p_dev, VL53L8CX_APIs_ResultsData* data){
	do
	{
		__WFI();	// Wait for interrupt
				if(IntCount !=0 ){
					IntCount=0;

		  		// Get the sensor's ranging data
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
		  		printf("n");
		  		// Loop to print data of all 16 regions
		  		for (int i = 0; i < resolution; i++) {
		  			printf("Zone : %3d, Status : %3u, Distance : %4d mm, Signal : %5lu kcps/SPADs\r\n",
					 i,
					 data->target_status[VL53L8CX_NB_TARGET_PER_ZONE * i],
					 data->distance_mm[VL53L8CX_NB_TARGET_PER_ZONE * i],
					 data->signal_per_spad[VL53L8CX_NB_TARGET_PER_ZONE * i]);
		  		}
		  	}
	}
	while(1);

}

/**********************************************************************/

void get_motion_indicator(VL53L8CX_APIs_ResultsData* data){
	do{
		status = vl53l8cx_check_data_ready(&Dev, &p_data_ready);

		if(p_data_ready)
		{
			vl53l8cx_get_resolution(&Dev, &resolution);
			vl53l8cx_get_ranging_data(&Dev, &Results);

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
				data->motion_indicator.motion[motion_config.map_id[i]] = Results.motion_indicator.motion[motion_config.map_id[i]];
			}

			/* As the sensor is set in 4x4 mode by default, we have a total
			 * of 16 zones to print. For this example, only the data of first zone are
			 * print */
			printf("Print data no : %3u\n", Dev.streamcount);
			for(int i = 0; i < 16; i++)
			{
				printf("Zone : %3d, Motion power : %3lu\n",
					i,
					data->motion_indicator.motion[motion_config.map_id[i]]);
			}
			printf("\n");
		}

		/* Wait a few ms to avoid too high polling (function in platform
		 * file, not in API) */
		delay(5);
	}while(1);
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

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetResolution(uint8_t res){

	if(vl53l8cx_set_resolution(&Dev, res))
		return VL53L8CX_ERR_RES;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t freq){

	if(vl53l8cx_set_ranging_frequency_hz(&Dev, freq))	// Set 5Hz ranging frequency
		return VL53L8CX_ERR_Freq;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetRangingMode(uint8_t rangMode){
	vl53l8cx_set_ranging_mode(&Dev, rangMode);
	return VL53L8CX_OK;

}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetPowerMode(uint8_t pwrMode){
	vl53l8cx_set_power_mode(&Dev, pwrMode);
		return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SampleRanging(VL53L8CX_APIs_ResultsData* data){

	if(vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	if (is_interrupt) {
		get_data_by_interrupt(&Dev, data);
	}
	else {
		get_data_by_polling(&Dev, data);
	}
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_GetIntegrationTime(uint32_t* integration_time_ms){
	if(vl53l8cx_get_integration_time_ms(&Dev, integration_time_ms))
		return VL53L8CX_ERR_INTEG_TIME;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetIntegrationTime(uint32_t integration_time_ms){
	if(vl53l8cx_set_integration_time_ms(&Dev, integration_time_ms))
		return VL53L8CX_ERR_INTEG_TIME;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_Detection_Thresholds(VL53L8CX_APIs_ResultsData* data)
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


			get_data_throshold(&Dev, data);

			return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_StopRanging(void){
	if(vl53l8cx_stop_ranging(&Dev))
			return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetSharpener(uint8_t sharpener){
	if(vl53l8cx_set_sharpener_percent(&Dev, sharpener))
		return VL53L8CX_ERR_SHARPENER;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_GetSharpener(uint8_t* sharpener){
	vl53l8cx_get_sharpener_percent(&Dev, sharpener);
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_Calibration(){
	if(vl53l8cx_calibrate_xtalk(&Dev, 3, 4, 600))
		return VL53L8CX_ERR_CALIBRATE;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_GetCalibrationData(uint8_t* pDataCalibrate){
	if(vl53l8cx_get_caldata_xtalk(&Dev, xtalk_data))
		return VL53L8CX_ERR_CALIBRATE;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetCalibrationData(uint8_t* pDataCalibrate){
	if(vl53l8cx_get_caldata_xtalk(&Dev, xtalk_data))
			return VL53L8CX_ERR_CALIBRATE;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_VisualizeXtalk(void){
	uint32_t i, j;
	union Block_header *bh_ptr;
	uint32_t xtalk_signal_kcps_grid[VL53L8CX_RESOLUTION_8X8];
	uint16_t xtalk_shape_bins[144];

	/* Swap buffer */
	VL53L8CX_SwapBuffer(xtalk_data, VL53L8CX_XTALK_BUFFER_SIZE);

	/* Get data */
	for(i = 0; i < VL53L8CX_XTALK_BUFFER_SIZE; i = i + 4)
	{
		bh_ptr = (union Block_header *)&(xtalk_data[i]);
		if (bh_ptr->idx == 0xA128){
			printf("Xtalk shape bins located at position %#06x\n", (int)i);
			for (j = 0; j < 144; j++){
				memcpy(&(xtalk_shape_bins[j]), &(xtalk_data[i + 4 + j * 2]), 2);
				printf("xtalk_shape_bins[%d] = %d\n", (int)j, (int)xtalk_shape_bins[j]);
			}
		}
		if (bh_ptr->idx == 0x9FFC){
			printf("Xtalk signal kcps located at position %#06x\n", (int)i);
			for (j = 0; j < VL53L8CX_RESOLUTION_8X8; j++){
				memcpy(&(xtalk_signal_kcps_grid[j]), &(xtalk_data[i + 4 + j * 4]), 4);
				xtalk_signal_kcps_grid[j] /= 2048;
				printf("xtalk_signal_kcps_grid[%d] = %d\n", (int)j, (int)xtalk_signal_kcps_grid[j]);
			}
		}
	}
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_MotionIndicator(VL53L8CX_APIs_ResultsData* data){
	/* Create motion indicator with resolution 4x4 */
	status = vl53l8cx_motion_indicator_init(&Dev, &motion_config, VL53L8CX_RESOLUTION_4X4);
	if(status)
	{
		printf("Motion indicator init failed with status : %u\n", status);
		return status;
	}

	/* (Optional) Change the min and max distance used to detect motions. The
	 * difference between min and max must never be >1500mm, and minimum never be <400mm,
	 * otherwise the function below returns error 127 */
	status = vl53l8cx_motion_indicator_set_distance_motion(&Dev, &motion_config, 1000, 2000);
	if(status)
	{
		printf("Motion indicator set distance motion failed with status : %u\n", status);
		return status;
	}

	/* If user want to change the resolution, he also needs to update the motion indicator resolution */
	//status = vl53l8cx_set_resolution(&Dev, VL53L8CX_RESOLUTION_4X4);
	//status = vl53l8cx_motion_indicator_set_resolution(&Dev, &motion_config, VL53L8CX_RESOLUTION_4X4);

	/* Increase ranging frequency for the example */
	status = vl53l8cx_set_ranging_frequency_hz(&Dev, 2);

	status = vl53l8cx_start_ranging(&Dev);

	get_motion_indicator(data);

	return VL53L8CX_OK;

}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SYNCRanging(VL53L8CX_APIs_ResultsData* data){
	vl53l8cx_set_external_sync_pin_enable(&Dev, 1);
	if(vl53l8cx_start_ranging(&Dev))
			return VL53L8CX_ERR_Rang;
	if (is_interrupt) {
		get_data_by_interrupt(&Dev, data);
	}
	else {
		get_data_by_polling(&Dev, data);
	}

	return VL53L8CX_OK;
}

VL53L8CX_Status VL53L8CX_GetResolution(uint8_t *res){
	status = vl53l8cx_check_data_ready(&Dev, &p_data_ready);
	if(p_data_ready)
	{
		vl53l8cx_get_resolution(&Dev, &res);
	}

	return VL53L8CX_OK;

}
