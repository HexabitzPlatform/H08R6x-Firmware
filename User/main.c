/*
 BitzOS (BOS) V0.3.6 - Copyright (C) 2017-2024 Hexabitz
 All rights reserved

 File Name     : main.c
 Description   : Main program body.
 */
/* Includes ------------------------------------------------------------------*/
#include "BOS.h"

int16_t distance[16],average,motion[16],numoftarget[16];
uint8_t f;
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Main function ------------------------------------------------------------*/

int main(void) {

	Module_Init();		//Initialize Module &  BitzOS

	//Don't place your code here.
	for (;;) {
	}
}

/*-----------------------------------------------------------*/

/* User Task */
void UserTask(void *argument) {
	uint8_t c = 0;
	c = c + 1;
	c = 15;

	//StreamToTerminal(2, AVERAGE, 10, 20000);

	// put your code here, to run repeatedly.
	while (1) {
		if(f==1)
		{
			SampleDistance(distance);
		}
		if(f==2)
		{
			SampleDistanceAverage(&average);

		}
		if(f==3)
		{
			MotionIndicator(motion);
		}
		if(f==4)
		{
			NumberOfTargets(numoftarget);
		}




	}
}

/*-----------------------------------------------------------*/
