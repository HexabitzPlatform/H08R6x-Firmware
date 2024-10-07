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
//#include "H08R6.h"
/* Exported Type's instance  ---------------------------------------------*/
int status;
volatile int IntCount;
VL53L8CX_Configuration Dev; /* Sensor configuration */
VL53L8CX_ResultsData Results; /* Results data from VL53L8CX */
uint8_t resolution, isAlive, p_data_ready;
uint32_t elapsedTime;

VL53L8CX_Motion_Configuration motion_config; /* Motion configuration*/
VL53L8CX_DetectionThresholds thresholds[VL53L8CX_NB_THRESHOLDS];
uint8_t xtalk_data[VL53L8CX_XTALK_BUFFER_SIZE];

/* Local Functions Definitions */
VL53L8CX_Status VL53L8CX_Reset(void);
VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t Freq);
VL53L8CX_Status VL53L8CX_GetIntegrationTime(uint32_t *Integration_time_ms);
VL53L8CX_Status VL53L8CX_SetIntegrationTime(uint32_t Integration_time_ms);
VL53L8CX_Status VL53L8CX_SetSharpener(uint8_t Sharpener);
VL53L8CX_Status VL53L8CX_GetSharpener(uint8_t *Sharpener);
//VL53L8CX_Status VL53L8CX_Detection_Thresholds(VL53L8CX_APIs_ResultsData *Data);
//VL53L8CX_Status VL53L8CX_SYNCRanging(VL53L8CX_APIs_ResultsData *Data);
VL53L8CX_Status VL53L8CX_StopRanging(void);

void get_data_by_polling(VL53L8CX_Configuration *p_dev);
void get_data_by_interrupt(VL53L8CX_Configuration *p_dev);



/* Platform Exported Functions ********************************************/
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == INT_Pin) {
		IntCount++;
	}
}

/**************************************************************************/
/* Local Functions  *******************************************************/
/**************************************************************************/

/**
 * @brief get reading data with interrupt mode after starting ranging
 * @param1: (VL53L8CX_Configuration) *p_dev :  VL53L8CX Configuration structure.
 */
void get_data_by_interrupt(VL53L8CX_Configuration *p_dev) {
	do {
		__WFI();	// Wait for interrupt
		if (IntCount != 0) {
			IntCount = 0;
			status = vl53l8cx_get_resolution(p_dev, &resolution);
			status = vl53l8cx_get_ranging_data(p_dev, &Results);
			break;
		}
		_DELAY_MS(5);
	} while (1);
}

/**********************************************************************/
/**
 * @brief get reading data with polling mode after starting ranging
 * @param1: (VL53L8CX_Configuration) *p_dev :  VL53L8CX Configuration structure.
 */
void get_data_by_polling(VL53L8CX_Configuration *p_dev) {
	do {
		status = vl53l8cx_check_data_ready(&Dev, &p_data_ready);

		if (p_data_ready) {
			status = vl53l8cx_get_resolution(p_dev, &resolution);
			status = vl53l8cx_get_ranging_data(p_dev, &Results);
			break;
		} else {
			_DELAY_MS(5);
		}
	} while (1);

}

/**********************************************************************/
/**
 * @brief Perform an hardware reset of the sensor and initialize the sensor
 * to load the firmware into the VL53L8CX. It takes a few hundred milliseconds.
 * @return (uint8_t) status : 0 if OK
 */
VL53L8CX_Status VL53L8CX_Reset(void) {

	/* Reset VL53L8CX sensor */
	VL53L8CX_Reset_Sensor(&(Dev.platform));

	/* Check if there is a VL53L8CX sensor connected */
	if (vl53l8cx_is_alive(&Dev, &isAlive))
		return VL53L8CX_ERR_INIT;
	if (!isAlive)
		return VL53L8CX_ERR_INIT;

	/* Init VL53L8CX sensor */
	if (vl53l8cx_init(&Dev))
		return VL53L8CX_ERR_INIT;

	return VL53L8CX_OK;

}

/**********************************************************************/
/**
 * @brief Set new ranging frequency in Hz. Ranging frequency
 * corresponds to the measurements frequency. This setting depends of
 * the resolution, so please select your resolution before using this function.
 * @param1 (uint8_t) frequency_hz : Contains the ranging frequency in Hz.
 * - For 4x4, min and max allowed values are : [1:60]
 * - For 8x8, min and max allowed values are : [1:15]
 * @return (uint8_t) status : 0 if ranging frequency is OK
 */
VL53L8CX_Status VL53L8CX_SetFrequancy(uint8_t Freq) {

	if (vl53l8cx_set_ranging_frequency_hz(&Dev, Freq))// Set 5Hz ranging frequency
		return VL53L8CX_ERR_Freq;
	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief Gets the current integration time in ms.
 * @param1 (uint32_t) *Integration_time_ms: Contains integration time in ms.
 * @return (uint8_t) status : 0 if integration time is OK.
 */
VL53L8CX_Status VL53L8CX_GetIntegrationTime(uint32_t *Integration_time_ms) {
	if (vl53l8cx_get_integration_time_ms(&Dev, Integration_time_ms))
		return VL53L8CX_ERR_INTEG_TIME;
	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief Set a new integration time in ms. Integration time must
 * be computed to be lower than the ranging period, for a selected resolution.
 * Please note that this function has no impact on ranging mode continous.
 * @param1 (uint32_t) Integration_time_ms : Contains the integration time in ms. For all
 * resolutions and frequency, the minimum value is 2ms, and the maximum is
 * 1000ms.
 * @return (uint8_t) status : 0 if set integration time is OK.
 */
VL53L8CX_Status VL53L8CX_SetIntegrationTime(uint32_t Integration_time_ms) {
	if (vl53l8cx_set_integration_time_ms(&Dev, Integration_time_ms))
		return VL53L8CX_ERR_INTEG_TIME;
	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief Set a new sharpener value in percent. Sharpener can be
 * changed to blur more or less zones depending of the application. Min value is
 * 0 (disabled), and max is 99.
 * @param1 (uint32_t) Sharpener : Value between 0 (disabled) and 99%.
 * @return (uint8_t) status : 0 if set sharpener is OK.
 */
VL53L8CX_Status VL53L8CX_SetSharpener(uint8_t Sharpener) {
	if (vl53l8cx_set_sharpener_percent(&Dev, Sharpener))
		return VL53L8CX_ERR_SHARPENER;
	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief Get the current sharpener in percent.
 * @param1 (uint32_t) *Sharpener: Contains the sharpener in percent.
 * @return (uint8_t) status : 0 if get sharpener is OK.
 */
VL53L8CX_Status VL53L8CX_GetSharpener(uint8_t *Sharpener) {
	vl53l8cx_get_sharpener_percent(&Dev, Sharpener);
	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief Programming the detection thresholds. It create 2 thresholds
 * per zone for a 4x4 resolution.
 * @param1 Result data for sensor VL53L8CX_APIs_ResultsData *Data.
 * @return (uint8_t) status : 0 if programming is OK
 */
//VL53L8CX_Status VL53L8CX_Detection_Thresholds(VL53L8CX_APIs_ResultsData *Data) {
//	/* Set all values to 0 */
//	memset(&thresholds, 0, sizeof(thresholds));
//
//	/* Add thresholds for all zones (16 zones in resolution 4x4, or 64 in 8x8) */
//	for (int i = 0; i < 16; i++) {
//		/* The first wanted thresholds is GREATER_THAN mode. Please note that the
//		 * first one must always be set with a mathematic_operation
//		 * VL53L8CX_OPERATION_NONE.
//		 * For this example, the signal thresholds is set to 150 kcps/spads
//		 * (the format is automatically updated inside driver)
//		 */
//		thresholds[2 * i].zone_num = i;
//		thresholds[2 * i].measurement = VL53L8CX_SIGNAL_PER_SPAD_KCPS;
//		thresholds[2 * i].type = VL53L8CX_GREATER_THAN_MAX_CHECKER;
//		thresholds[2 * i].mathematic_operation = VL53L8CX_OPERATION_NONE;
//		thresholds[2 * i].param_low_thresh = 1400;
//		thresholds[2 * i].param_high_thresh = 1500;
//
//		/* The second wanted checker is IN_WINDOW mode. We will set a
//		 * mathematical thresholds VL53L8CX_OPERATION_OR, to add the previous
//		 * checker to this one.
//		 * For this example, distance thresholds are set between 200mm and
//		 * 400mm (the format is automatically updated inside driver).
//		 */
//		thresholds[2 * i + 1].zone_num = i;
//		thresholds[2 * i + 1].measurement = VL53L8CX_DISTANCE_MM;
//		thresholds[2 * i + 1].type = VL53L8CX_IN_WINDOW;
//		thresholds[2 * i + 1].mathematic_operation = VL53L8CX_OPERATION_OR;
//		thresholds[2 * i + 1].param_low_thresh = 200;
//		thresholds[2 * i + 1].param_high_thresh = 400;
//	}
//	/* The last thresholds must be clearly indicated. As we have 32
//	 * checkers (16 zones x 2), the last one is the 31 */
//	thresholds[31].zone_num = VL53L8CX_LAST_THRESHOLD | thresholds[31].zone_num;
//
//	/* Send array of thresholds to the sensor */
//	vl53l8cx_set_detection_thresholds(&Dev, thresholds);
//
//	/* Enable detection thresholds */
//	vl53l8cx_set_detection_thresholds_enable(&Dev, 1);
//
//	status = vl53l8cx_set_ranging_frequency_hz(&Dev, 10);
//
//	IntCount = 0;
//	status = vl53l8cx_start_ranging(&Dev);
//
//	if (IS_INTERRUPT) {
//		get_data_by_interrupt(&Dev);
//	} else {
//		get_data_by_polling(&Dev);
//	}
//	for (int i = 0; i < resolution; i++) {
//		Data->distance_mm[i] = Results.distance_mm[i];
//		Data->range_sigma_mm[i] = Results.range_sigma_mm[i];
//		Data->reflectance[i] = Results.reflectance[i];
//		Data->target_status[i] = Results.target_status[i];
//		Data->nb_target_detected[i] = Results.nb_target_detected[i];
//		Data->signal_per_spad[i] = Results.signal_per_spad[i];
//		Data->ambient_per_spad[i] = Results.ambient_per_spad[i];
//		Data->nb_spads_enabled[i] = Results.nb_spads_enabled[i];
//	}
//	Data->silicon_temp_degc = Results.silicon_temp_degc;
//
//	return VL53L8CX_OK;
//}

/**********************************************************************/
/**
 * @brief Start ranging when external synchronizing pin is enabled.
 * @param1: (VL53L8CX_APIs_ResultsData) *Data : VL53L8CX Results structure.
 * @return: (uint8_t) status : 0 if start is OK.
 */
//VL53L8CX_Status VL53L8CX_SYNCRanging(VL53L8CX_APIs_ResultsData *Data) {
//	vl53l8cx_set_external_sync_pin_enable(&Dev, 1);
//	if (vl53l8cx_start_ranging(&Dev))
//		return VL53L8CX_ERR_Rang;
//	if (IS_INTERRUPT) {
//		get_data_by_interrupt(&Dev);
//	} else {
//		get_data_by_polling(&Dev);
//	}
//	for (int i = 0; i < resolution; i++) {
//		Data->distance_mm[i] = Results.distance_mm[i];
//		Data->range_sigma_mm[i] = Results.range_sigma_mm[i];
//		Data->reflectance[i] = Results.reflectance[i];
//		Data->target_status[i] = Results.target_status[i];
//		Data->nb_target_detected[i] = Results.nb_target_detected[i];
//		Data->signal_per_spad[i] = Results.signal_per_spad[i];
//		Data->ambient_per_spad[i] = Results.ambient_per_spad[i];
//		Data->nb_spads_enabled[i] = Results.nb_spads_enabled[i];
//	}
//	Data->silicon_temp_degc = Results.silicon_temp_degc;
//
//	return VL53L8CX_OK;
//}

/**********************************************************************/
/**
 * @brief Stop the ranging session. It must be used when the
 * sensor streams, after calling vl53l8cx_start_ranging().
 * @return (uint8_t) status : 0 if stop is OK
 */
VL53L8CX_Status VL53L8CX_StopRanging(void) {
	if (vl53l8cx_stop_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;
}

/**************************************************************************/
/* Exported functions  ****************************************************/
/**************************************************************************/
/*
 * @brief: reset and initialize VL53L8CX sensor with default
 * configurations which is 4x4 Resolution, Autonomous ranging mode
 * and wake up power mode.
 * @return: status
 */
VL53L8CX_Status VL53L8CX_Init(void) {

	if (VL53L8CX_Reset())
		return VL53L8CX_ERR_INIT;

	if (VL53L8CX_SetResolution(ZONES_4X4))
		return VL53L8CX_ERR_RES;

	if (VL53L8CX_SetFrequancy(VL53L8CX_APIs_FREQUANCY))
		return VL53L8CX_ERR_Freq;

	if (VL53L8CX_SetRangingMode(AUTONOMOUS))
		return VL53L8CX_ERR_Rang;

	if (VL53L8CX_SetPowerMode(WAKEUP))
		return VL53L8CX_ERR_PWR;

	return VL53L8CX_OK;

}

/**********************************************************************/
/*
 * @brief: set resolution for sensor from two available choices: 4*4 and 8*8
 * @param11: enum ZONES_4x4 or ZONES_8x8 to set resolution
 * @return: status
 * WARNING : As others settings depend to this one, it must be the first to use.
 */
VL53L8CX_Status VL53L8CX_SetResolution(Resolution_e Res) {

	if (Res == ZONES_4X4) {
		if (vl53l8cx_set_resolution(&Dev, VL53L8CX_RESOLUTION_4X4))
			return VL53L8CX_ERR_RES;
	} else if (Res == ZONES_8X8) {
		if (vl53l8cx_set_resolution(&Dev, VL53L8CX_RESOLUTION_8X8))
			return VL53L8CX_ERR_RES;
	}

	if (VL53L8CX_SetFrequancy(VL53L8CX_APIs_FREQUANCY))
		return VL53L8CX_ERR_Freq;

	return VL53L8CX_OK;

}

/**********************************************************************/
/**
 * @brief: Control power mode of sensor. Please ensure that the device
 * is not streaming before calling the function.
 * @param1: enum WAKEUP, SLEEP or DEEP_SLEEP.
 * @return: (uint8_t) status : 0 if power mode is OK.
 */
VL53L8CX_Status VL53L8CX_SetPowerMode(PwrMode_e Pwr) {

	if (Pwr == WAKEUP) {
		if (vl53l8cx_set_power_mode(&Dev, VL53L8CX_POWER_MODE_WAKEUP))
			return VL53L8CX_ERR_PWR;
	} else if (Pwr == SLEEP) {
		if (vl53l8cx_set_power_mode(&Dev, VL53L8CX_POWER_MODE_SLEEP))
			return VL53L8CX_ERR_PWR;
	} else if (Pwr == DEEP_SLEEP) {
		if (vl53l8cx_set_power_mode(&Dev, VL53L8CX_POWER_MODE_DEEP_SLEEP))
			return VL53L8CX_ERR_PWR;
	}
	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief: Set the ranging mode of sensor. Two modes are
 * available using ULD : Continuous and autonomous.
 * @param1: enum AUTONOMOUS or CONTINUOUS.
 * @return: (uint8_t) status : 0 if set ranging mode is OK.
 */
VL53L8CX_Status VL53L8CX_SetRangingMode(RangingMode_e Rang) {
	if (Rang == AUTONOMOUS) {
		if (vl53l8cx_set_ranging_mode(&Dev, VL53L8CX_RANGING_MODE_AUTONOMOUS))
			return VL53L8CX_ERR_Rang;
	} else if (Rang == CONTINUOUS) {
		if (vl53l8cx_set_ranging_mode(&Dev, VL53L8CX_RANGING_MODE_CONTINUOUS))
			return VL53L8CX_ERR_Rang;
	}

	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief Perform calibration Xtalk.
 * This calibration is recommended is user wants to use a coverglass.
 * Conditions : There should be black painted board at distance in 600mm
 * and the target must stay in Full FOV, so short distance are easier for calibration.
 * @return (uint8_t) status : 0 if calibration OK.
 */
VL53L8CX_Status VL53L8CX_xTalkCalibration(void) {
	if (vl53l8cx_calibrate_xtalk(&Dev, 3, 4, 600))
		return VL53L8CX_ERR_CALIBRATE;
	if (vl53l8cx_get_caldata_xtalk(&Dev, xtalk_data))
		return VL53L8CX_ERR_CALIBRATE;
	if (vl53l8cx_set_caldata_xtalk(&Dev, xtalk_data))
		return VL53L8CX_ERR_CALIBRATE;
	return VL53L8CX_OK;
}

/**********************************************************************/
/**
 * @brief: This function gets sample of a ranging distance.
 * @param1: (VL53L8CX_APIs_Distance) *Distance : VL53L8CX distance structure.
 * @return: (uint8_t) status : 0 if start is OK.
 */
VL53L8CX_Status VL53L8CX_SampleDistance(int16_t *Distance) {
	if (vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	if (IS_INTERRUPT) {
		get_data_by_interrupt(&Dev);
	} else {
		get_data_by_polling(&Dev);
	}
	for (int i = 0; i < resolution; i++) {
		Distance[i] = Results.distance_mm[i];
	}
	if (VL53L8CX_StopRanging())
		return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;

}

/**********************************************************************/
/**
 * @brief: This function gets sample of an average ranging distance.
 * @param1: int16_t *Distance_a : pointer for distance variable.
 * @return: (uint8_t) status : 0 if start is OK.
 */
VL53L8CX_Status VL53L8CX_SampleDistanceAverage(int16_t *Distance_a) {
	if (vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;
	if (IS_INTERRUPT) {
		get_data_by_interrupt(&Dev);
	} else {
		get_data_by_polling(&Dev);
	}

	*Distance_a = ((Results.distance_mm[5] + Results.distance_mm[6]
			+ Results.distance_mm[9] + Results.distance_mm[10]) / 4);
	if (VL53L8CX_StopRanging())
		return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;

}

/**********************************************************************/
/**
 * @brief: This function streams an average ranging distance for a specific time in seconds.
 * @param1: int16_t *Distance_a : pointer for distance variable.
 * @param2: uint32_t Time_out : time of streaming.
 * @return: (uint8_t) status : 0 if start is OK.
 */
VL53L8CX_Status VL53L8CX_StreamDistance(int16_t *Distance_a, uint32_t Time_out) {
	uint32_t startTime, end;

	if (vl53l8cx_start_ranging(&Dev))
		return VL53L8CX_ERR_Rang;

	if (IS_INTERRUPT) {
		startTime = _TIME_MS();
		end = _TIME_MS();
		elapsedTime = (end - startTime) / 10;
		while (elapsedTime < Time_out) {
			get_data_by_interrupt(&Dev);
			*Distance_a = ((Results.distance_mm[5] + Results.distance_mm[6]
					+ Results.distance_mm[9] + Results.distance_mm[10]) / 4);
			end = _TIME_MS();
			elapsedTime = (end - startTime) / 1000;

		}
	} else {
		startTime = _TIME_MS();
		end = _TIME_MS();
		elapsedTime = (end - startTime) / 10;
		while (elapsedTime < Time_out) {
			get_data_by_polling(&Dev);
			*Distance_a = ((Results.distance_mm[5] + Results.distance_mm[6]
					+ Results.distance_mm[9] + Results.distance_mm[10]) / 4);
			end = _TIME_MS();
			elapsedTime = (end - startTime) / 1000;
		}

	}
	if (VL53L8CX_StopRanging())
		return VL53L8CX_ERR_Rang;
	return VL53L8CX_OK;
}

/**********************************************************************/
///**
// * @brief: Number of valid target detected for 1 zone.
// * @param1: VL53L8CX_APIs_NOfTargets* NofTargets:  Number of valid target variable.
// * @return: (uint8_t) status : 0 if start is OK.
// */
//VL53L8CX_Status VL53L8CX_NumberofTargets(int16_t *NofTargets) {
//	if (vl53l8cx_start_ranging(&Dev))
//		return VL53L8CX_ERR_Rang;
//	if (IS_INTERRUPT) {
//		get_data_by_interrupt(&Dev);
//	} else {
//		get_data_by_polling(&Dev);
//	}
//	for (int i = 0; i < resolution; i++) {
//		NofTargets[i] = Results.nb_target_detected[i];
//	}
//	if (VL53L8CX_StopRanging())
//		return VL53L8CX_ERR_Rang;
//	return VL53L8CX_OK;
//
//}

/**********************************************************************/
/**
 * @brief: Perform motion indicator to detect motion in specific range in mm between
 * LOW_MOTION_INDICATOR and HIGH_MOTION_INDICATOR where we can change the min and max
 * distance used to detect motions. The difference between min and max must never be >1500mm,
 * and minimum never be <400mm, Default used resolution is 4x4. for using this function make
 * sure that macro VL53L8CX_DISABLE_MOTION_INDICATOR is NOT enabled.
 * @param1: VL53L8CX_APIs_Indicator* Indicator:  Indicator array contains 1 in zone where motion is detected.
 * @return: (uint8_t) status : 0 if start is OK.
 */
//VL53L8CX_Status VL53L8CX_MotionIndicator(int16_t *Indicator) {
//	/* Create motion indicator with resolution 4x4 */
//	if (vl53l8cx_motion_indicator_init(&Dev, &motion_config,
//			VL53L8CX_RESOLUTION_4X4))
//		return VL53L8CX_ERR_MOTION_IND;
//
//	/* (Optional) Change the min and max distance used to detect motions. The
//	 * difference between min and max must never be >1500mm, and minimum never be <400mm,
//	 * otherwise the function below returns error 127 */
//	status = vl53l8cx_motion_indicator_set_distance_motion(&Dev, &motion_config,
//			LOW_MOTION_INDICATOR, HIGH_MOTION_INDICATOR);
//	if (status) {
//		return VL53L8CX_ERR_MOTION_IND;
//	}
//
//	/* If user want to change the resolution, he also needs to update the motion indicator resolution */
//
//	if (vl53l8cx_get_resolution(&Dev, &resolution))
//		return VL53L8CX_ERR_RES;
//
//	if (vl53l8cx_motion_indicator_set_resolution(&Dev, &motion_config,
//			resolution))
//		return VL53L8CX_ERR_MOTION_IND;
//
//	if (vl53l8cx_start_ranging(&Dev))
//		return VL53L8CX_ERR_Rang;
//
//	if (IS_INTERRUPT) {
//		get_data_by_interrupt(&Dev);
//	} else {
//		get_data_by_polling(&Dev);
//	}
//
//	for (int i = 0; i < resolution; i++) {
//		if (Results.motion_indicator.motion[motion_config.map_id[i]] >= 44) {
//			//printf("Motion detected in this area: %3d \n", i);
//			Indicator[i] = 1;
//		}
//	}
//	if (VL53L8CX_StopRanging())
//		return VL53L8CX_ERR_Rang;
//
//	return VL53L8CX_OK;
//}

/**********************************************************************/
/**
 * @brief: This function gets a sample of all sensor ranging readings.
 * @param1: (VL53L8CX_APIs_ResultsData) *Data : VL53L8CX Results structure.
 * @return: (uint8_t) status : 0 if start is OK.
 */
//VL53L8CX_Status VL53L8CX_SampleRangingAllData(VL53L8CX_APIs_ResultsData *Data) {
//
//	if (vl53l8cx_start_ranging(&Dev))
//		return VL53L8CX_ERR_Rang;
//	if (IS_INTERRUPT) {
//		get_data_by_interrupt(&Dev);
//	} else {
//		get_data_by_polling(&Dev);
//	}
//	for (int i = 0; i < resolution; i++) {
//		Data->distance_mm[i] = Results.distance_mm[i];
//		Data->range_sigma_mm[i] = Results.range_sigma_mm[i];
//		Data->reflectance[i] = Results.reflectance[i];
//		Data->target_status[i] = Results.target_status[i];
//		Data->nb_target_detected[i] = Results.nb_target_detected[i];
//		Data->signal_per_spad[i] = Results.signal_per_spad[i];
//		Data->ambient_per_spad[i] = Results.ambient_per_spad[i];
//		Data->nb_spads_enabled[i] = Results.nb_spads_enabled[i];
//	}
//	Data->silicon_temp_degc = Results.silicon_temp_degc;
//
//	if (VL53L8CX_StopRanging())
//		return VL53L8CX_ERR_Rang;
//	return VL53L8CX_OK;
//}

/************************ (C) COPYRIGHT Hexabitz *****END OF FILE****/

