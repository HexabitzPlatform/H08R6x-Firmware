/*
 BitzOS (BOS) V0.3.6 - Copyright (C) 2017-2024 Hexabitz
 All rights reserved

 File Name     : H08R6.c
 Description   : Source code for module H08R6.
 (Description_of_module)

 (Description of Special module peripheral configuration):
 >>
 >>
 >>

 */

/* Includes ------------------------------------------------------------------*/
#include "BOS.h"
#include "H08R6_inputs.h"
#include "VL53L8CX_APIs.h"
#include <math.h>

/* Define UART variables */
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart6;
/* Exported variables */
//extern FLASH_ProcessTypeDef pFlash;
extern uint8_t numOfRecordedSnippets;

/* Driver variables */
// int16_t average;

/* variables for Streams ----------------------------------------------------*/
uint32_t numofsamples[2], Timeout[2];
uint8_t Port[2], Module[2], mode[2];
uint8_t tofMode;

/* Private variables ---------------------------------------------------------*/
TaskHandle_t TOFTaskHandle = NULL;
static bool stopStream = false;
uint8_t StopeCliStreamFlag;

int16_t H08R6_average = 0;

/* Module exported parameters ------------------------------------------------*/
ModuleParam_t ModuleParam[NUM_MODULE_PARAMS] = { { .ParamPtr = &H08R6_average, .ParamFormat = FMT_INT16, .ParamName = "average" } };




typedef void (*SampleToString)(char*,size_t);
typedef void (*SampleMemsToBuffer)(int16_t *buffer);

/* Private function prototypes -----------------------------------------------*/
void TOFTask(void *argument);

Module_Status Exporttoport(uint8_t module, uint8_t port, All_Data function);
Module_Status Exportstreamtoport(uint8_t module,uint8_t port,All_Data function,uint32_t Numofsamples,uint32_t timeout);
Module_Status Exportstreamtoterminal(uint8_t Port,All_Data function,uint32_t Numofsamples,uint32_t timeout);

/* Local functions */
static Module_Status PollingSleepCLISafe(uint32_t period, long Numofsamples);
static Module_Status StreamToCLI(uint32_t Numofsamples,uint32_t timeout,SampleToString function);

static Module_Status StreamMemsToBuf(int16_t *Buffer, uint32_t Numofsamples,uint32_t timeout,SampleMemsToBuffer function);
void SampleAverageBuf(int16_t *buffer);

void FLASH_Page_Eras(uint32_t Addr);
void ExecuteMonitor(void);


/* Create CLI commands --------------------------------------------------------*/
static portBASE_TYPE SampleSensorCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);


/* CLI command structure : sample */
const CLI_Command_Definition_t SampleCommandDefinition = {
	(const int8_t *) "sample",
	(const int8_t *) "sample:\r\n Syntax: sample [Average].\r\n\r\n",
	SampleSensorCommand,
	1
};
/* CLI command structure : demo */

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* ---------------------------------------------------------------------
 |							 Private Functions	                	   |
 ----------------------------------------------------------------------- 
 */

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow : 
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 48000000
 *            HCLK(Hz)                       = 48000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 1
 *            HSE Frequency(Hz)              = 8000000
 *            PREDIV                         = 1
 *            PLLMUL                         = 6
 *            Flash Latency(WS)              = 1
 * @param  None
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
	RCC_OscInitStruct.PLL.PLLN = 12;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

	/** Initializes the peripherals clocks
	 */
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC
			| RCC_PERIPHCLK_USART2;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_TIM1;
	PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLKSOURCE_PCLK1;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	__SYSCFG_CLK_ENABLE()
	;

	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

}

/*-----------------------------------------------------------*/

/***************************************************************************/
/* Save Command Topology in Flash RO */
uint8_t SaveTopologyToRO(void) {

	HAL_StatusTypeDef flashStatus = HAL_OK;

	/* flashAdd is initialized with 8 because the first memory room in topology page
	 * is reserved for module's ID */
	uint16_t flashAdd = 8;
	uint16_t temp = 0;

	/* Unlock the FLASH control register access */
	HAL_FLASH_Unlock();

	/* Erase Topology page */
	FLASH_PageErase(FLASH_BANK_2, TOPOLOGY_PAGE_NUM);

	/* Wait for an Erase operation to complete */
	flashStatus = FLASH_WaitForLastOperation((uint32_t) HAL_FLASH_TIMEOUT_VALUE);

	if (flashStatus != HAL_OK) {
		/* return FLASH error code */
		return pFlash.ErrorCode;
	}

	else {
		/* Operation is completed, disable the PER Bit */
		CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
	}

	/* Save module's ID and topology */
	if (myID) {

		/* Save module's ID */
		temp = (uint16_t) (N << 8) + myID;

		/* Save module's ID in Flash memory */
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, TOPOLOGY_START_ADDRESS, temp);

		/* Wait for a Write operation to complete */
		flashStatus = FLASH_WaitForLastOperation((uint32_t) HAL_FLASH_TIMEOUT_VALUE);

		if (flashStatus != HAL_OK) {
			/* return FLASH error code */
			return pFlash.ErrorCode;
		}

		else {
			/* If the program operation is completed, disable the PG Bit */
			CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
		}

		/* Save topology */
		for (uint8_t row = 1; row <= N; row++) {
			for (uint8_t column = 0; column <= MAX_NUM_OF_PORTS; column++) {
				/* Check the module serial number
				 * Note: there isn't a module has serial number 0
				 */
				if (Array[row - 1][0]) {
					/* Save each element in topology Array in Flash memory */
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, TOPOLOGY_START_ADDRESS + flashAdd,
							Array[row - 1][column]);
					/* Wait for a Write operation to complete */
					flashStatus = FLASH_WaitForLastOperation((uint32_t) HAL_FLASH_TIMEOUT_VALUE);
					if (flashStatus != HAL_OK) {
						/* return FLASH error code */
						return pFlash.ErrorCode;
					} else {
						/* If the program operation is completed, disable the PG Bit */
						CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
						/* update new flash memory address */
						flashAdd += 8;
					}
				}
			}
		}
	}
	/* Lock the FLASH control register access */
	HAL_FLASH_Lock();
}

/***************************************************************************/
/* Save Command Snippets in Flash RO */
uint8_t SaveSnippetsToRO(void) {
	HAL_StatusTypeDef FlashStatus = HAL_OK;
	uint8_t snipBuffer[sizeof(Snippet_t) + 1] = { 0 };

	/* Unlock the FLASH control register access */
	HAL_FLASH_Unlock();
	/* Erase Snippets page */
	FLASH_PageErase(FLASH_BANK_2, SNIPPETS_PAGE_NUM);
	/* Wait for an Erase operation to complete */
	FlashStatus = FLASH_WaitForLastOperation((uint32_t) HAL_FLASH_TIMEOUT_VALUE);

	if (FlashStatus != HAL_OK) {
		/* return FLASH error code */
		return pFlash.ErrorCode;
	} else {
		/* Operation is completed, disable the PER Bit */
		CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
	}

	/* Save Command Snippets */
	int currentAdd = SNIPPETS_START_ADDRESS;
	for (uint8_t index = 0; index < NumOfRecordedSnippets; index++) {
		/* Check if Snippet condition is true or false */
		if (Snippets[index].Condition.ConditionType) {
			/* A marker to separate Snippets */
			snipBuffer[0] = 0xFE;
			memcpy((uint32_t*) &snipBuffer[1], (uint8_t*) &Snippets[index], sizeof(Snippet_t));
			/* Copy the snippet struct buffer (20 x NumOfRecordedSnippets). Note this is assuming sizeof(Snippet_t) is even */
			for (uint8_t j = 0; j < (sizeof(Snippet_t) / 4); j++) {
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, currentAdd, *(uint64_t*) &snipBuffer[j * 8]);
				FlashStatus = FLASH_WaitForLastOperation((uint32_t) HAL_FLASH_TIMEOUT_VALUE);
				if (FlashStatus != HAL_OK) {
					return pFlash.ErrorCode;
				} else {
					/* If the program operation is completed, disable the PG Bit */
					CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
					currentAdd += 8;
				}
			}
			/* Copy the snippet commands buffer. Always an even number. Note the string termination char might be skipped */
			for (uint8_t j = 0; j < ((strlen(Snippets[index].CMD) + 1) / 4); j++) {
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, currentAdd, *(uint64_t*) (Snippets[index].CMD + j * 4));
				FlashStatus = FLASH_WaitForLastOperation((uint32_t) HAL_FLASH_TIMEOUT_VALUE);
				if (FlashStatus != HAL_OK) {
					return pFlash.ErrorCode;
				} else {
					/* If the program operation is completed, disable the PG Bit */
					CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
					currentAdd += 8;
				}
			}
		}
	}
	/* Lock the FLASH control register access */
	HAL_FLASH_Lock();
}

/* Clear Array topology in SRAM and Flash RO */
uint8_t ClearROtopology(void) {
	/* Clear the Array */
	memset(Array, 0, sizeof(Array));
	N = 1;
	myID = 0;

	return SaveTopologyToRO();
}
/*-----------------------------------------------------------*/

/* --- Trigger ST factory bootloader update for a remote module.
 */
void remoteBootloaderUpdate(uint8_t src, uint8_t dst, uint8_t inport,
		uint8_t outport) {

	uint8_t myOutport = 0, lastModule = 0;
	int8_t *pcOutputString;

	/* 1. Get route to destination module */
	myOutport = FindRoute(myID, dst);
	if (outport && dst == myID) { /* This is a 'via port' update and I'm the last module */
		myOutport = outport;
		lastModule = myID;
	} else if (outport == 0) { /* This is a remote update */
		if (NumberOfHops(dst)== 1)
		lastModule = myID;
		else
		lastModule = Route[NumberOfHops(dst)-1]; /* previous module = route[Number of hops - 1] */
	}

	/* 2. If this is the source of the message, show status on the CLI */
	if (src == myID) {
		/* Obtain the address of the output buffer.  Note there is no mutual
		 exclusion on this buffer as it is assumed only one command console
		 interface will be used at any one time. */
		pcOutputString = FreeRTOS_CLIGetOutputBuffer();

		if (outport == 0)		// This is a remote module update
			sprintf((char*) pcOutputString, pcRemoteBootloaderUpdateMessage,
					dst);
		else
			// This is a 'via port' remote update
			sprintf((char*) pcOutputString,
					pcRemoteBootloaderUpdateViaPortMessage, dst, outport);

		strcat((char*) pcOutputString, pcRemoteBootloaderUpdateWarningMessage);
		writePxITMutex(inport, (char*) pcOutputString,
				strlen((char*) pcOutputString), cmd50ms);
		Delay_ms(100);
	}

	/* 3. Setup my inport and outport for bootloader update */
	SetupPortForRemoteBootloaderUpdate(inport);
	SetupPortForRemoteBootloaderUpdate(myOutport);

	/* 5. Build a DMA stream between my inport and outport */
	StartScastDMAStream(inport, myID, myOutport, myID, BIDIRECTIONAL,
			0xFFFFFFFF, 0xFFFFFFFF, false);
}
/***************************************************************************/
/* Trigger ST factory bootloader update for a remote module */
void RemoteBootloaderUpdate(uint8_t src, uint8_t dst, uint8_t inport, uint8_t outport) {

	uint8_t myOutport = 0, lastModule = 0;
	int8_t *pcOutputString;

	/* 1. Get Route to destination module */
	myOutport = FindRoute(myID, dst);
	if (outport && dst == myID) { /* This is a 'via port' update and I'm the last module */
		myOutport = outport;
		lastModule = myID;
	} else if (outport == 0) { /* This is a remote update */
		if (NumberOfHops(dst)== 1)
		lastModule = myID;
		else
		lastModule = Route[NumberOfHops(dst)-1]; /* previous module = Route[Number of hops - 1] */
	}

	/* 2. If this is the source of the message, show status on the CLI */
	if (src == myID) {
		/* Obtain the address of the output buffer.  Note there is no mutual
		 * exclusion on this buffer as it is assumed only one command console
		 * interface will be used at any one time. */
		pcOutputString = FreeRTOS_CLIGetOutputBuffer();

		if (outport == 0)		// This is a remote module update
			sprintf((char*) pcOutputString, pcRemoteBootloaderUpdateMessage, dst);
		else
			// This is a 'via port' remote update
			sprintf((char*) pcOutputString, pcRemoteBootloaderUpdateViaPortMessage, dst, outport);

		strcat((char*) pcOutputString, pcRemoteBootloaderUpdateWarningMessage);
		writePxITMutex(inport, (char*) pcOutputString, strlen((char*) pcOutputString), cmd50ms);
		Delay_ms(100);
	}

	/* 3. Setup my inport and outport for bootloader update */
	SetupPortForRemoteBootloaderUpdate(inport);
	SetupPortForRemoteBootloaderUpdate(myOutport);

	/* 5. Build a DMA stream between my inport and outport */
	StartScastDMAStream(inport, myID, myOutport, myID, BIDIRECTIONAL, 0xFFFFFFFF, 0xFFFFFFFF, false);
}

/***************************************************************************/
/* Setup a port for remote ST factory bootloader update:
 * Set baudrate to 57600
 * Enable even parity
 * Set datasize to 9 bits
 */
void SetupPortForRemoteBootloaderUpdate(uint8_t port){

	UART_HandleTypeDef *huart =GetUart(port);
	HAL_UART_DeInit(huart);
	huart->Init.Parity = UART_PARITY_EVEN;
	huart->Init.WordLength = UART_WORDLENGTH_9B;
	HAL_UART_Init(huart);

	/* The CLI port RXNE interrupt might be disabled so enable here again to be sure */
	__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);

}
/*-----------------------------------------------------------*/

///* --- Setup a port for remote ST factory bootloader update:
// - Set baudrate to 57600
// - Enable even parity
// - Set datasize to 9 bits
// */
//void SetupPortForRemoteBootloaderUpdate(uint8_t port) {
//	UART_HandleTypeDef *huart = GetUart(port);
//
//	huart->Init.BaudRate = 57600;
//	huart->Init.Parity = UART_PARITY_EVEN;
//	huart->Init.WordLength = UART_WORDLENGTH_9B;
//	HAL_UART_Init(huart);
//
//	/* The CLI port RXNE interrupt might be disabled so enable here again to be sure */
//	__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
//}
/***************************************************************************/
/* Samples a module parameter value based on parameter index.
 * paramIndex: Index of the parameter (1-based index).
 * value: Pointer to store the sampled float value.
 */
Module_Status GetModuleParameter(uint8_t paramIndex, float *value) {
	Module_Status status = BOS_OK;

	switch (paramIndex) {

	/* Invalid parameter index */
	default:
		status = BOS_ERR_WrongParam;
		break;
	}

	return status;
}

/* enable stop mode regarding only UART1 , UART2 , and UART3 */
BOS_Status EnableStopModebyUARTx(uint8_t port) {

	UART_WakeUpTypeDef WakeUpSelection;
	UART_HandleTypeDef *huart = GetUart(port);

	if ((huart->Instance == USART1) || (huart->Instance == USART2) || (huart->Instance == USART3)) {

		/* make sure that no UART transfer is on-going */
		while (__HAL_UART_GET_FLAG(huart, USART_ISR_BUSY) == SET);

		/* make sure that UART is ready to receive */
		while (__HAL_UART_GET_FLAG(huart, USART_ISR_REACK) == RESET);

		/* set the wake-up event:
		 * specify wake-up on start-bit detection */
		WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;
		HAL_UARTEx_StopModeWakeUpSourceConfig(huart, WakeUpSelection);

		/* Enable the UART Wake UP from stop mode Interrupt */
		__HAL_UART_ENABLE_IT(huart, UART_IT_WUF);

		/* enable MCU wake-up by LPUART */
		HAL_UARTEx_EnableStopMode(huart);

		/* enter STOP mode */
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	} else
		return BOS_ERROR;

}

/***************************************************************************/
/* Enable standby mode regarding wake-up pins:
 * WKUP1: PA0  pin
 * WKUP4: PA2  pin
 * WKUP6: PB5  pin
 * WKUP2: PC13 pin
 * NRST pin
 *  */
BOS_Status EnableStandbyModebyWakeupPinx(WakeupPins_t wakeupPins) {

	/* Clear the WUF FLAG */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF);

	/* Enable the WAKEUP PIN */
	switch (wakeupPins) {

	case PA0_PIN:
		HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1); /* PA0 */
		break;

	case PA2_PIN:
		HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4); /* PA2 */
		break;

	case PB5_PIN:
		HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN6); /* PB5 */
		break;

	case PC13_PIN:
		HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2); /* PC13 */
		break;

	case NRST_PIN:
		/* do no thing*/
		break;
	}

	/* Enable SRAM content retention in Standby mode */
	HAL_PWREx_EnableSRAMRetention();

	/* Finally enter the standby mode */
	HAL_PWR_EnterSTANDBYMode();

	return BOS_OK;
}
/* --- H08R6 module initialization.
 */
void Module_Peripheral_Init(void) {

	/* Array ports */
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_USART4_UART_Init();
	MX_USART5_UART_Init();
	MX_USART6_UART_Init();
	MX_GPIO_Init();
	MX_SPI2_Init();


	//Circulating DMA Channels ON All Module
	for (int i = 1; i <= NUM_OF_PORTS; i++) {
		if (GetUart(i) == &huart1) {
			dmaIndex[i - 1] = &(DMA1_Channel1->CNDTR);
		} else if (GetUart(i) == &huart2) {
			dmaIndex[i - 1] = &(DMA1_Channel2->CNDTR);
		} else if (GetUart(i) == &huart3) {
			dmaIndex[i - 1] = &(DMA1_Channel3->CNDTR);
		} else if (GetUart(i) == &huart4) {
			dmaIndex[i - 1] = &(DMA1_Channel4->CNDTR);
		} else if (GetUart(i) == &huart5) {
			dmaIndex[i - 1] = &(DMA1_Channel5->CNDTR);
		} else if (GetUart(i) == &huart6) {
			dmaIndex[i - 1] = &(DMA1_Channel6->CNDTR);
		}
	}



	/* Create module special task (if needed) */
//	xTaskCreate(TOFTask,(const char* )"TOFTask",configMINIMAL_STACK_SIZE,NULL,osPriorityNormal - osPriorityIdle,&TOFTaskHandle);


}

/*-----------------------------------------------------------*/
/* --- H08R6 message processing task.
 */
Module_Status Module_MessagingTask(uint16_t code, uint8_t port, uint8_t src,
		uint8_t dst, uint8_t shift) {
	Module_Status result =H08R6_OK;
		uint32_t period =0, timeout =0;

		switch(code){
			case CODE_H08R7_SAMPLE_PORT: {
				Exporttoport(cMessage[port - 1][shift],cMessage[port - 1][1 + shift],AVERAGE);
				break;
			}
			default:
				result =H08R6_ERR_UnknownMessage;
				break;
		}

		return result;
}

/* --- Register this module CLI Commands
 */
void RegisterModuleCLICommands(void) {

	FreeRTOS_CLIRegisterCommand( &SampleCommandDefinition );

}

/* --- Get the port for a given UART. 
 */
uint8_t GetPort(UART_HandleTypeDef *huart) {

	if (huart->Instance == USART4)
		return P1;
	else if (huart->Instance == USART2)
		return P2;
	else if (huart->Instance == USART3)
		return P3;
	else if (huart->Instance == USART1)
		return P4;
	else if (huart->Instance == USART5)
		return P5;
	else if (huart->Instance == USART6)
		return P6;

	return 0;
}

/* Module special task function (if needed) */
//void TOFTask(void *argument) {
//
//	switch(tofMode){
//		case STREAM_TO_PORT:
//			Exportstreamtoport(Module[0],Port[0],mode[0],numofsamples[0],Timeout[0]);
//			break;
//		case STREAM_TO_Terminal:
//			Exportstreamtoterminal(Port[1],mode[1],numofsamples[1],Timeout[1]);
//			break;
//		default:
//			osDelay(10);
//			break;
//	}
//
//	taskYIELD();
//}

/*-----------------------------------------------------------*/
static Module_Status StreamMemsToBuf(int16_t *Buffer, uint32_t Numofsamples,uint32_t timeout,SampleMemsToBuffer function){

	Module_Status status = H08R6_OK;
	int16_t buffer;
	uint8_t coun;
	uint32_t period = timeout / Numofsamples;

	if (period < MIN_MEMS_PERIOD_MS)
		return H08R6_ERR_WrongParams;

	// TODO: Check if CLI is enable or not

	if (period > timeout)
		timeout = period;

	long numTimes = timeout / period;
	stopStream = false;

	while ((numTimes-- > 0) || (timeout >= MAX_MEMS_TIMEOUT_MS)) {
		function(&buffer);
		Buffer[coun] = buffer;
		coun++;
		vTaskDelay(pdMS_TO_TICKS(period));
		if (stopStream) {
			status = H08R6_ERR_TERMINATED;
			break;
		}
	}
	return status;


}

/*-----------------------------------------------------------*/
void SampleAverageBuf(int16_t *buffer){
	int16_t average_1;
	SampleDistanceAverage(&average_1);
//	buffer = Distance_average;
}
/*-----------------------------------------------------------*/
static Module_Status StreamToCLI(uint32_t Numofsamples,uint32_t timeout,SampleToString function){
	Module_Status status =H08R6_OK;
	int8_t *pcOutputString = NULL;
	uint32_t period =timeout / Numofsamples;
	if(period < MIN_MEMS_PERIOD_MS)
		return H08R6_ERR_WrongParams;

	// TODO: Check if CLI is enable or not
	for(uint8_t chr =0; chr < MSG_RX_BUF_SIZE; chr++){
		if(UARTRxBuf[pcPort - 1][chr] == '\r'){
			UARTRxBuf[pcPort - 1][chr] =0;
		}
	}
	if(1 == StopeCliStreamFlag){
		StopeCliStreamFlag =0;
		static char *pcOKMessage =(int8_t* )"Stop stream !\n\r";
		writePxITMutex(pcPort,pcOKMessage,strlen(pcOKMessage),10);
		return status;
	}
	if(period > timeout)
		timeout =period;

	long numTimes =timeout / period;
	stopStream = false;

	while((numTimes-- > 0) || (timeout >= MAX_MEMS_TIMEOUT_MS)){
		pcOutputString =FreeRTOS_CLIGetOutputBuffer();
		function((char* )pcOutputString,100);

		writePxMutex(pcPort,(char* )pcOutputString,strlen((char* )pcOutputString),cmd500ms,HAL_MAX_DELAY);
		if(PollingSleepCLISafe(period,Numofsamples) != H08R6_OK)
			break;
	}

	memset((char* )pcOutputString,0,configCOMMAND_INT_MAX_OUTPUT_SIZE);
	sprintf((char* )pcOutputString,"\r\n");
	return status;
}
/*-----------------------------------------------------------*/
Module_Status Exportstreamtoterminal(uint8_t Port,All_Data function,uint32_t Numofsamples,uint32_t timeout)
{
	Module_Status status = H08R6_OK;
	int8_t *pcOutputString = NULL;
	char cstring[100];
	int16_t average_d;
	uint32_t period = timeout / Numofsamples;
	if (period < MIN_MEMS_PERIOD_MS)
		return H08R6_ERR_WrongMode;

	switch (function) {
	case AVERAGE:

		if (period > timeout)
			timeout = period;

		stopStream = false;

		while ((Numofsamples-- > 0) || (timeout >= MAX_MEMS_TIMEOUT_MS)) {
			pcOutputString = FreeRTOS_CLIGetOutputBuffer();
			if ((status = SampleDistanceAverage(&average_d)) != H08R6_OK)
				return status;

			snprintf(cstring, 50, "Distance (mm) | %d\r\n", average_d);

			writePxMutex(Port, (char*) cstring, strlen((char*) cstring),
			cmd500ms, HAL_MAX_DELAY);
			if (PollingSleepCLISafe(period, Numofsamples) != H08R6_OK)
				break;
		}
		break;

	default:
		status = H08R6_ERR_WrongParams;
		break;
	}

	tofMode = DEFAULT;
	return status;
}


/*-----------------------------------------------------------*/
static Module_Status PollingSleepCLISafe(uint32_t period,long Numofsamples){
	const unsigned DELTA_SLEEP_MS =100; // milliseconds
	long numDeltaDelay =period / DELTA_SLEEP_MS;
	unsigned lastDelayMS =period % DELTA_SLEEP_MS;

	while(numDeltaDelay-- > 0){
		vTaskDelay(pdMS_TO_TICKS(DELTA_SLEEP_MS));

		// Look for ENTER key to stop the stream
		for(uint8_t chr =1; chr < MSG_RX_BUF_SIZE; chr++){
			if(UARTRxBuf[pcPort - 1][chr] == '\r'){
				UARTRxBuf[pcPort - 1][chr] =0;
				StopeCliStreamFlag = 1;
				return H08R6_ERR_TERMINATED;
			}
		}

		if(stopStream)
			return H08R6_ERR_TERMINATED;
	}

	vTaskDelay(pdMS_TO_TICKS(lastDelayMS));
	return H08R6_OK;
}
/*-----------------------------------------------------------*/

Module_Status Exportstreamtoport(uint8_t module,uint8_t port,All_Data function,uint32_t Numofsamples,uint32_t timeout){
	Module_Status status =H08R6_OK;
	uint32_t samples =0;
	uint32_t period =0;
	period =timeout / Numofsamples;

	if(timeout < MIN_PERIOD_MS || period < MIN_PERIOD_MS)
		return H08R6_ERR_WrongParams;

	while(samples < Numofsamples){
		status =Exporttoport(module,port,function);
		vTaskDelay(pdMS_TO_TICKS(period));
		samples++;
	}
	tofMode = DEFAULT;

	return status;
}

/*-----------------------------------------------------------*/
Module_Status Exporttoport(uint8_t module, uint8_t port, All_Data function) {

	int16_t average;
	static uint8_t temp[4] = { 0 };
	Module_Status status = H08R6_OK;

	switch (function) {
	case AVERAGE:

		if ((status = SampleDistanceAverage(&average)) != H08R6_OK)
			return status = H08R6_ERROR;

		if (module == myID || module == 0) {
			temp[0] = (uint8_t) ((*(uint32_t*) &average) >> 0);
			temp[1] = (uint8_t) ((*(uint32_t*) &average) >> 8);
			writePxITMutex(port, (char*) &temp[0], 2 * sizeof(uint8_t), 10);
		} else {
			if (H08R6_OK == status)
				MessageParams[1] = BOS_OK;
			else
				MessageParams[1] = BOS_ERROR;
			MessageParams[0] = FMT_UINT16;
			MessageParams[2] = 1;
			MessageParams[3] =
					(uint8_t) ((*(uint32_t*) &average) >> 0);
			MessageParams[4] =
					(uint8_t) ((*(uint32_t*) &average) >> 8);
			SendMessageToModule(module, CODE_READ_RESPONSE,
					2 * sizeof(uint8_t) + 3);
		}
		break;
	default:
		status = H08R6_ERR_WrongParams;
		break;
	}


	return status;
}

/* -----------------------------------------------------------------------
 |								  APIs                         |
 /* -----------------------------------------------------------------------
 */

/*-----------------------------------------------------------*/

Module_Status SampleDistanceAverage(int16_t *average) {
	Module_Status status = H08R6_OK;

	VL53L8CX_Init();




	if ((status = VL53L8CX_SampleDistanceAverage(average)) != H08R6_OK)
		return status = H08R6_ERROR;

	return status;
}

/*-----------------------------------------------------------*/
Module_Status SampletoPort(uint8_t module, uint8_t port, All_Data function) {
	Module_Status status = H08R6_OK;

	if (port == 0 && module == myID)
		return status = H08R6_ERR_WrongMode;

	Exporttoport(module, port, function);

	return status;
}
/*-----------------------------------------------------------*/
Module_Status StreamtoPort(uint8_t module,uint8_t port,All_Data function,uint32_t Numofsamples,uint32_t timeout){
	Module_Status status =H08R6_OK;

	if(port == 0 && module == myID)
		return status =H08R6_ERR_WrongMode;

	tofMode =STREAM_TO_PORT;
	Port[0] =port;
	Module[0] =module;
	numofsamples[0] =Numofsamples;
	Timeout[0] =timeout;
	mode[0] =function;
	return status;
}

/*-----------------------------------------------------------*/

Module_Status StreamToTerminal(uint8_t port,All_Data function,uint32_t Numofsamples,uint32_t timeout){
	Module_Status status =H08R6_OK;

	if(0 == port)
		return status =H08R6_ERR_WrongMode;

	tofMode =STREAM_TO_Terminal;
	Port[1] =port;
	numofsamples[1] =Numofsamples;
	Timeout[1] =timeout;
	mode[1] =function;
	return status;
}

/*-----------------------------------------------------------*/

Module_Status StreamToBuffer(int16_t *buffer,All_Data function,uint32_t Numofsamples,uint32_t timeout){
	switch(function){
		case AVERAGE:
			return StreamMemsToBuf(buffer,Numofsamples,timeout,SampleAverageBuf);
			break;
		default:
			break;
	}

}

/* -----------------------------------------------------------------------
 |								Commands							      |
 -----------------------------------------------------------------------
 */
static portBASE_TYPE SampleSensorCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
	const char *const AvrCmdName = "avr";



	const char *pSensName = NULL;
	portBASE_TYPE sensNameLen = 0;

	// Make sure we return something
	*pcWriteBuffer = '\0';

	pSensName = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &sensNameLen);

	if (pSensName == NULL) {
		snprintf((char *)pcWriteBuffer, xWriteBufferLen, "Invalid Arguments\r\n");
		return pdFALSE;
	}

	do {
		if (!strncmp(pSensName, AvrCmdName, strlen(AvrCmdName))) {
			Exportstreamtoterminal(pcPort,AVERAGE,1,500);

		}
		else {
			snprintf((char *)pcWriteBuffer, xWriteBufferLen, "Invalid Arguments\r\n");
		}

		return pdFALSE;
	} while (0);

	snprintf((char *)pcWriteBuffer, xWriteBufferLen, "Error reading Sensor\r\n");
	return pdFALSE;
}
/*-----------------------------------------------------------*/
// Port Mode => false and CLI Mode => true
static bool StreamCommandParser(const int8_t *pcCommandString, const char **ppSensName, portBASE_TYPE *pSensNameLen,
														bool *pPortOrCLI, uint32_t *pPeriod, uint32_t *pTimeout, uint8_t *pPort, uint8_t *pModule)
{
	const char *pPeriodMSStr = NULL;
	const char *pTimeoutMSStr = NULL;

	portBASE_TYPE periodStrLen = 0;
	portBASE_TYPE timeoutStrLen = 0;

	const char *pPortStr = NULL;
	const char *pModStr = NULL;

	portBASE_TYPE portStrLen = 0;
	portBASE_TYPE modStrLen = 0;

	*ppSensName = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, pSensNameLen);
	pPeriodMSStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 2, &periodStrLen);
	pTimeoutMSStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 3, &timeoutStrLen);

	// At least 3 Parameters are required!
	if ((*ppSensName == NULL) || (pPeriodMSStr == NULL) || (pTimeoutMSStr == NULL))
		return false;

	// TODO: Check if Period and Timeout are integers or not!
	*pPeriod = atoi(pPeriodMSStr);
	*pTimeout = atoi(pTimeoutMSStr);
	*pPortOrCLI = true;

	pPortStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 4, &portStrLen);
	pModStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 5, &modStrLen);

	if ((pModStr == NULL) && (pPortStr == NULL))
		return true;
	if ((pModStr == NULL) || (pPortStr == NULL))	// If user has provided 4 Arguments.
		return false;

	*pPort = atoi(pPortStr);
	*pModule = atoi(pModStr);
	*pPortOrCLI = false;

	return true;
}
/*-----------------------------------------------------------*/


/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
