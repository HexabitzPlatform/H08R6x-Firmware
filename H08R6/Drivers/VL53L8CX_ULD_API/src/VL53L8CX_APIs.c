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
VL53L8CX_Configuration 	Dev;		/* Sensor configuration */
VL53L8CX_ResultsData Results;		/* Results data from VL53L8CX */
uint8_t resolution, isAlive;
VL53L8CX_Motion_Configuration 	motion_config;
uint16_t idx;
VL53L8CX_DetectionThresholds thresholds[VL53L8CX_NB_THRESHOLDS];
uint8_t					xtalk_data[VL53L8CX_XTALK_BUFFER_SIZE];
#define is_interrupt 0

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
void get_data_by_polling(VL53L8CX_Configuration *p_dev);
void get_data_by_interrupt(VL53L8CX_Configuration *p_dev);


/**************************************************************************/
/* Platform Exported Functions ********************************************/
/**************************************************************************/

void delay(uint32_t ms){

	HAL_Delay(ms);

}

/**************************************************************************/
/* Local Functions  *******************************************************/
/**************************************************************************/

void get_data_by_interrupt(VL53L8CX_Configuration *p_dev){
	do
	{
		__WFI();	// Wait for interrupt
		if(IntCount !=0 ){
			IntCount=0;
			status = vl53l8cx_get_resolution(p_dev, &resolution);
			status = vl53l8cx_get_ranging_data(p_dev, &Results);
			break;
		}
		delay(5);
	}while(1);
}

/**********************************************************************/

void get_data_by_polling(VL53L8CX_Configuration *p_dev){
	do
	{
		status = vl53l8cx_check_data_ready(&Dev, &p_data_ready);

		if(p_data_ready){
			status = vl53l8cx_get_resolution(p_dev, &resolution);
			status = vl53l8cx_get_ranging_data(p_dev, &Results);
			break;
		}else{
			delay(5);
		}
	}
	while(1);

}


/**********************************************************************/

VL53L8CX_Status VL53L8CX_Reset(void){

	/* Reset VL53L8CX sensor */
	VL53L8CX_Reset_Sensor(&(Dev.platform));

	/* Check if there is a VL53L8CX sensor connected */
	if(vl53l8cx_is_alive(&Dev, &isAlive))
		return VL53L8CX_ERR_INIT;
	if(!isAlive)
		return VL53L8CX_ERR_INIT;

	/* Init VL53L8CX sensor */
	if(vl53l8cx_init(&Dev))
		return VL53L8CX_ERR_INIT;

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

			if (is_interrupt) {
				get_data_by_interrupt(&Dev);
			}
			else {
				get_data_by_polling(&Dev);
			}

			return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SYNCRanging(VL53L8CX_APIs_ResultsData* data){
	vl53l8cx_set_external_sync_pin_enable(&Dev, 1);
	if(vl53l8cx_start_ranging(&Dev))
			return VL53L8CX_ERR_Rang;
	if (is_interrupt) {
		get_data_by_interrupt(&Dev);
	}
	else {
		get_data_by_polling(&Dev);
	}
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


	return VL53L8CX_OK;
}

/**************************************************************************/
/* Exported functions  ****************************************************/
/**************************************************************************/

/*
 * @brief: initialize VL53L8CX sensor and reseting it
 * @retval: status
 */
VL53L8CX_Status VL53L8CX_Init(void){

	if(VL53L8CX_Reset())
		return VL53L8CX_ERR_INIT;

	if(VL53L8CX_SetResolution(RES_4_BY_4))
		return VL53L8CX_ERR_RES;

	if(VL53L8CX_SetFrequancy(VL53L8CX_APIs_FREQUANCY))
		return VL53L8CX_ERR_Freq;

	if(VL53L8CX_SetRangingMode(AUTONOMOUS))
		return VL53L8CX_ERR_Rang;

	if(VL53L8CX_SetPowerMode(WAKEUP))
		return VL53L8CX_ERR_PWR;

	return VL53L8CX_OK;

}

/*
 * @brief: set resolution for sensor from two available choices: 4*4 and 8*8
 * @param1: enum to set resolution
 * @retval: status
 * WARNING : As others settings depend to this one, it must be the first to use.
 */
VL53L8CX_Status VL53L8CX_SetResolution(Resolution_e res){

	if(res == RES_4_BY_4)
	{
		if(vl53l8cx_set_resolution(&Dev, VL53L8CX_RESOLUTION_4X4))
				return VL53L8CX_ERR_RES;
	}
	else if(res == RES_8_BY_8)
	{
		if(vl53l8cx_set_resolution(&Dev, VL53L8CX_RESOLUTION_8X8))
				return VL53L8CX_ERR_RES;
	}

	if(VL53L8CX_SetFrequancy(VL53L8CX_APIs_FREQUANCY))
			return VL53L8CX_ERR_Freq;

	return VL53L8CX_OK;

}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetPowerMode(PwrMode_e pwr){

	if(pwr == WAKEUP)
	{
		if(vl53l8cx_set_power_mode(&Dev, VL53L8CX_POWER_MODE_WAKEUP))
				return VL53L8CX_ERR_PWR;
	}
	else if(pwr == SLEEP)
	{
		if(vl53l8cx_set_power_mode(&Dev, VL53L8CX_POWER_MODE_SLEEP))
				return VL53L8CX_ERR_PWR;
	}
	else if(pwr == DEEP_SLEEP)
	{
		if(vl53l8cx_set_power_mode(&Dev, VL53L8CX_POWER_MODE_DEEP_SLEEP))
				return VL53L8CX_ERR_PWR;
	}
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SetRangingMode(RangingMode_e rang){
	if(rang == AUTONOMOUS)
	{
		if(vl53l8cx_set_ranging_mode(&Dev, VL53L8CX_RANGING_MODE_AUTONOMOUS))
				return VL53L8CX_ERR_Rang;
	}
	else if(rang == CONTINUOUS)
	{
		if(vl53l8cx_set_ranging_mode(&Dev, VL53L8CX_RANGING_MODE_CONTINUOUS))
				return VL53L8CX_ERR_Rang;
	}

	return VL53L8CX_OK;
}

/* Set ranging frequency to 10Hz.
 * Using 4x4, min frequency is 1Hz and max is 60Hz
 * Using 8x8, min frequency is 1Hz and max is 15Hz
 */

VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t freq){

	if(vl53l8cx_set_ranging_frequency_hz(&Dev, freq))	// Set 5Hz ranging frequency
		return VL53L8CX_ERR_Freq;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SampleDistance(VL53L8CX_APIs_Distance* Distance)
{
	if(vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	if (is_interrupt) {
		get_data_by_interrupt(&Dev);
	}
	else {
		get_data_by_polling(&Dev);
	}
	for(int i = 0; i < resolution;i++)
	{
		Distance->distance[i] = Results.distance_mm[i];
	}
	if(VL53L8CX_StopRanging())
			return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;

}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_NumberofTargets(VL53L8CX_APIs_NOfTargets* NofTargets)
{
	if(vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	if (is_interrupt) {
		get_data_by_interrupt(&Dev);
	}
	else {
		get_data_by_polling(&Dev);
	}
	for(int i = 0; i < resolution;i++)
	{
		NofTargets->nb_target[i]=Results.nb_target_detected[i];
	}
	if(VL53L8CX_StopRanging())
		return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;

}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_MotionIndicator(VL53L8CX_APIs_Indicator* Indicator){
	/* Create motion indicator with resolution 4x4 */
	if(vl53l8cx_motion_indicator_init(&Dev, &motion_config, VL53L8CX_RESOLUTION_4X4))
		return VL53L8CX_ERR_MOTION_IND;

	/* (Optional) Change the min and max distance used to detect motions. The
	 * difference between min and max must never be >1500mm, and minimum never be <400mm,
	 * otherwise the function below returns error 127 */
	status = vl53l8cx_motion_indicator_set_distance_motion(&Dev, &motion_config, LOW_MOTION_INDICATOR, HIGH_MOTION_INDICATOR);
	if(status)
	{
		return VL53L8CX_ERR_MOTION_IND;
	}

	/* If user want to change the resolution, he also needs to update the motion indicator resolution */

/*	if(vl53l8cx_get_resolution(&Dev, &resolution))
		return VL53L8CX_ERR_RES;

	if(vl53l8cx_motion_indicator_set_resolution(&Dev, &motion_config, resolution))
		return VL53L8CX_ERR_MOTION_IND;

	if(VL53L8CX_SetFrequancy(VL53L8CX_APIs_FREQUANCY))
			return VL53L8CX_ERR_Freq;
*/
	if(vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;

	if (is_interrupt) {
	get_data_by_interrupt(&Dev);
	}
	else {
		get_data_by_polling(&Dev);
	}

	for(int i = 0; i < resolution;i++)
	{
		if(Results.motion_indicator.motion[motion_config.map_id[i]] >= 44)
		{
			//printf("Motion detected in this area: %3d \n", i);
			Indicator->indicator[i] = 1;
		}
	}
	if(VL53L8CX_StopRanging())
				return VL53L8CX_ERR_Rang;

	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_SampleRangingAllData(VL53L8CX_APIs_ResultsData* data){

	if(vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	if (is_interrupt) {
		get_data_by_interrupt(&Dev);
	}
	else {
		get_data_by_polling(&Dev);
	}
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

	if(VL53L8CX_StopRanging())
				return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;
}

/**********************************************************************/

VL53L8CX_Status VL53L8CX_StopRanging(void){
	if(vl53l8cx_stop_ranging(&Dev))
			return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;
}
