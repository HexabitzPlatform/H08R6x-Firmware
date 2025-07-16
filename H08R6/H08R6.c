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

/* Includes ****************************************************************/
#include "BOS.h"
#include "VL53L8CX_APIs.h"

/* Exported Typedef ******************************************************/
/* Define UART variables */
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart6;

All_Data PortFunction;
All_Data TerminalFunction;

TimerHandle_t xTimerStream = NULL;
TaskHandle_t TOFTaskHandle = NULL;

/* Private Variables *******************************************************/
uint8_t flag;
int16_t Average = 0, Distance[16] = {0}, Motion[16] = {0}, NumOfTargets[16] = {0};
/* Streaming variables *****************************************************/
static bool stopstream = false;         /* Flag to indicate whether to stop streaming process */
uint8_t PortModule = 0u;                /* Module ID for the destination port */
uint8_t PortNumber = 0u;                /* Physical port number used for streaming */
uint8_t StreamMode = 0u;                /* Current active streaming mode (to port, terminal, etc.) */
uint8_t TerminalPort = 0u;              /* Port number used to output data to a terminal */
uint8_t StopeCliStreamFlag = 0u;        /* Flag to request stopping a CLI stream operation */
uint32_t SampleCount = 0u;              /* Counter to track the number of samples streamed */
uint32_t PortNumOfSamples = 0u;         /* Total number of samples to be sent through the port */
uint32_t TerminalNumOfSamples = 0u;     /* Total number of samples to be streamed to the terminal */

/* Global variables for sensor data used in ModuleParam */
int16_t H08R6_average = 0;

/* Module Parameters */
ModuleParam_t ModuleParam[NUM_MODULE_PARAMS] = { { .ParamPtr = &H08R6_average, .ParamFormat = FMT_INT16, .ParamName = "average" } };

/* Local Typedef related to stream functions */
typedef void (*SampleToString)(char*,size_t);
typedef void (*SampleToBuffer)(int16_t *buffer);

/* Private function prototypes *********************************************/
Module_Status GetModuleParameter(uint8_t paramIndex, float *value);
Module_Status Module_MessagingTask(uint16_t code,uint8_t port,uint8_t src,uint8_t dst,uint8_t shift);
BOS_Status EnableStopModebyUARTx(uint8_t port);
BOS_Status EnableStandbyModebyWakeupPinx(WakeupPins_t wakeupPins);
BOS_Status DisableStandbyModeWakeupPinx(WakeupPins_t wakeupPins);
uint8_t ClearROtopology(void);
uint8_t GetPort(UART_HandleTypeDef *huart);
void RemoteBootloaderUpdate(uint8_t src,uint8_t dst,uint8_t inport,uint8_t outport);
void SetupPortForRemoteBootloaderUpdate(uint8_t port);
void Module_Peripheral_Init(void);
void RegisterModuleCLICommands(void);

/* Local Functions ********************************************************/
Module_Status SampleToPort(uint8_t dstModule, uint8_t dstPort, All_Data dataFunction);
Module_Status SampleToTerminal(uint8_t dstPort,All_Data dataFunction);
Module_Status StreamToPort(uint8_t dstModule,uint8_t dstPort,All_Data dataFunction,uint32_t numOfSamples,uint32_t streamTimeout);
Module_Status StreamToTerminal(uint8_t dstPort,All_Data dataFunction,uint32_t numOfSamples,uint32_t streamTimeout);
Module_Status StreamToBuffer(int16_t *buffer,All_Data function, uint32_t Numofsamples, uint32_t timeout);
static Module_Status PollingSleepCLISafe(uint32_t period, long Numofsamples);
static Module_Status StreamToCLI(uint32_t Numofsamples,uint32_t timeout,SampleToString function);
static Module_Status StreamToBuf(int16_t *buffer,uint32_t Numofsamples,uint32_t timeout,SampleToBuffer function);
void StreamTimeCallback(TimerHandle_t xTimerStream);
void SampleDistanceToString(char *cstring, size_t maxLen);
void SampleDistanceAverageToString(char *cstring, size_t maxLen);
void MotionIndicatorToString(char *cstring, size_t maxLen);
void NummberOfTargetsToString(char *cstring, size_t maxLen);
void StopStream(void);
void SampleDistanceBuf(int16_t *buffer);
void SampleDistanceAverageBuf(int16_t *buffer);
void NumberOfTargetsBuf(int16_t *buffer);
void MotionIndicatorBuf(int16_t *buffer);
/* General Function ********************************************************************/
Module_Status SampleDistance(int16_t *distance);
Module_Status SampleDistanceAverage(int16_t *average);
Module_Status MotionIndicator(int16_t *indicator);
Module_Status NummberOfTargets(int16_t *numOfTargets);

/* Create CLI commands *****************************************************/
static portBASE_TYPE SampleTOFCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);
static portBASE_TYPE StreamTOFCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString);

static bool StreamCommandParser(const int8_t *pcCommandString, const char **ppSensName, portBASE_TYPE *pSensNameLen,bool *pPortOrCLI, uint32_t *pPeriod, uint32_t *pTimeout, uint8_t *pPort, uint8_t *pModule);

void TOF(void *argument);
/* CLI command structure ***************************************************/
/* CLI command structure : sample */
const CLI_Command_Definition_t SampleCommandDefinition = {
	(const int8_t *) "sample",
	(const int8_t *) "sample:\r\n Syntax: parameters : 1-[distance]/[average]/[motion]/[numoftargets] 2-[cli or port] 3-[port if param 2 is port] 4-[module if param 2 is port]..\r\n\r\n",
	SampleTOFCommand,
	-1
};

/* CLI command structure : sample */
const CLI_Command_Definition_t StreamCommandDefinition = {
	(const int8_t *) "stream",
	(const int8_t *) "stream:\r\n Syntax: parameters :  1-[distance]/[average]/[motion]/[numoftargets] 2-[number of samples] 3-[timeout in ms] 4-[cli or port] 5-[port if param 4 is port] 6-[module if param 4 is port].\r\n\r\n",
	StreamTOFCommand,
	-1
};


/***************************************************************************/
/************************ Private function Definitions *********************/
/***************************************************************************/
/* @brief  System Clock Configuration
 *         This function configures the system clock as follows:
 *            - System Clock source            = PLL (HSE)
 *            - SYSCLK(Hz)                     = 64000000
 *            - HCLK(Hz)                       = 64000000
 *            - AHB Prescaler                  = 1
 *            - APB1 Prescaler                 = 1
 *            - HSE Frequency(Hz)              = 8000000
 *            - PLLM                           = 1
 *            - PLLN                           = 16
 *            - PLLP                           = 2
 *            - Flash Latency(WS)              = 2
 *            - Clock Source for UART1,UART2,UART3 = 16MHz (HSI)
 */
void SystemClock_Config(void){
	RCC_OscInitTypeDef RCC_OscInitStruct ={0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct ={0};

	/** Configure the main internal regulator output voltage */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE; // Enable both HSI and HSE oscillators
	RCC_OscInitStruct.HSEState = RCC_HSE_ON; // Enable HSE (External High-Speed Oscillator)
	RCC_OscInitStruct.HSIState = RCC_HSI_ON; // Enable HSI (Internal High-Speed Oscillator)
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1; // No division on HSI
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT; // Default calibration value for HSI
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON; // Enable PLL
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE; // Set PLL source to HSE
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1; // Prescaler for PLL input
	RCC_OscInitStruct.PLL.PLLN =16; // Multiplication factor for PLL
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // PLLP division factor
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2; // PLLQ division factor
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2; // PLLR division factor
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/** Initializes the CPU, AHB and APB buses clocks */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // Select PLL as the system clock source
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1; // AHB Prescaler set to 1
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1; // APB1 Prescaler set to 1

	HAL_RCC_ClockConfig(&RCC_ClkInitStruct,FLASH_LATENCY_2); // Configure system clocks with flash latency of 2 WS
}

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

/***************************************************************************/
/* Clear Array topology in SRAM and Flash RO */
uint8_t ClearROtopology(void) {
	/* Clear the Array */
	memset(Array, 0, sizeof(Array));
	N = 1;
	myID = 0;

	return SaveTopologyToRO();
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

/***************************************************************************/
/* This functions is useful only for input (sensors) modules.
 * Samples a module parameter value based on parameter index.
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

/***************************************************************************/
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

/***************************************************************************/
/* Disable standby mode regarding wake-up pins:
 * WKUP1: PA0  pin
 * WKUP4: PA2  pin
 * WKUP6: PB5  pin
 * WKUP2: PC13 pin
 * NRST pin
 *  */
BOS_Status DisableStandbyModeWakeupPinx(WakeupPins_t wakeupPins){

	/* The standby wake-up is same as a system RESET:
	 * The entire code runs from the beginning just as if it was a RESET.
	 * The only difference between a reset and a STANDBY wake-up is that, when the MCU wakes-up,
	 * The SBF status flag in the PWR power control/status register (PWR_CSR) is set */
	if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET){
		/* clear the flag */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

		/* Disable  Wake-up Pinx */
		switch(wakeupPins){

			case PA0_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1); /* PA0 */
				break;

			case PA2_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN4); /* PA2 */
				break;

			case PB5_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN6); /* PB5 */
				break;

			case PC13_PIN:
				HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2); /* PC13 */
				break;

			case NRST_PIN:
				/* do no thing*/
				break;
		}

		IND_blink(1000);

	}
	else
		return BOS_OK;

}

/***************************************************************************/
/* H08R6 module initialization */
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
	VL53L8CX_Status status = VL53L8CX_Init();

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
	xTaskCreate(TOF, (const char*) "TOF", configMINIMAL_STACK_SIZE, NULL, osPriorityNormal - osPriorityIdle,
			&TOFTaskHandle);
	xTimerStream =xTimerCreate("StreamTimer",pdMS_TO_TICKS(1000),pdTRUE,(void* )1,StreamTimeCallback);
}

/***************************************************************************/
/* H08R6 message processing task */
Module_Status Module_MessagingTask(uint16_t code, uint8_t port, uint8_t src,
		uint8_t dst, uint8_t shift) {
	Module_Status result =H08R6_OK;
		uint32_t period =0, timeout =0;

		switch(code){
			case CODE_H08R7_SAMPLE_DISTANCE: {
				SampleToPort(cMessage[port - 1][shift],cMessage[port - 1][1 + shift],DISTANCE);
				break;
			}
			case CODE_H08R7_SAMPLE_DISTANCE_AVRG: {
				SampleToPort(cMessage[port - 1][shift],cMessage[port - 1][1 + shift],AVERAGE);
				break;
			}
			case CODE_H08R7_MOTION_INDICATOR: {
				SampleToPort(cMessage[port - 1][shift],cMessage[port - 1][1 + shift],MOTION);
				break;
			}
			case CODE_H08R7_NUM_OF_TARGETS: {
				SampleToPort(cMessage[port - 1][shift],cMessage[port - 1][1 + shift],NUM_OF_TARGET);
				break;
			}
			default:
				result =H08R6_ERR_UNKNOWNMESSAGE;
				break;
		}

		return result;
}

/***************************************************************************/
/* Register this module CLI Commands */
void RegisterModuleCLICommands(void) {

	FreeRTOS_CLIRegisterCommand( &SampleCommandDefinition );
	FreeRTOS_CLIRegisterCommand( &StreamCommandDefinition );

}

/***************************************************************************/
/* Get the port for a given UART */
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



/***************************************************************************/
/**************************** Local Functions ***************************/
/***************************************************************************/
static Module_Status PollingSleepCLISafe(uint32_t period, long Numofsamples) {
	const unsigned DELTA_SLEEP_MS = 100; // milliseconds
	long numDeltaDelay = period / DELTA_SLEEP_MS;
	unsigned lastDelayMS = period % DELTA_SLEEP_MS;

	while (numDeltaDelay-- > 0) {
		vTaskDelay(pdMS_TO_TICKS(DELTA_SLEEP_MS));

		/* Look for ENTER key to stop the stream */
		for (uint8_t chr = 1; chr < MSG_RX_BUF_SIZE; chr++) {
			if (UARTRxBuf [pcPort - 1] [chr] == '\r') {
				UARTRxBuf [pcPort - 1] [chr] = 0;
				flag = 1;
				return H08R6_ERR_TERMINATED;
			}
		}

		if (stopstream)
			return H08R6_ERR_TERMINATED;
	}

	vTaskDelay(pdMS_TO_TICKS(lastDelayMS));
	return H08R6_OK;
}
uint8_t tof;
uint8_t streamFlag,endStreamFlag;
uint8_t dstModule,dstPort;
All_Data dataFunction;
uint32_t numOfSamples,streamTimeout;
void TOF(void *argument) {

	/* Infinite loop */
	for (;;) {

			if(streamFlag == 1){
				 SampleToTerminal(dstPort, dataFunction);
				 streamFlag = 0;
			}
			else if(streamFlag == 2){
				 SampleToPort(dstModule, dstPort, dataFunction);
				 streamFlag = 0;
			}
			else if(streamFlag == 3){
				 StreamToTerminal(dstPort, dataFunction, numOfSamples, streamTimeout);
				 streamFlag = 0;
			}
			else if(streamFlag == 4){
				 StreamToPort(dstModule, dstPort, dataFunction, numOfSamples, streamTimeout);
				 streamFlag = 0;
			}

		}
		taskYIELD();

}
/***************************************************************************/
/*
 * @brief: Samples data and exports it to a specified port.
 * @param dstModule: The module number to export data from.
 * @param dstPort: The port number to export data to.
 * @param dataFunction: Function to sample data (e.g., HEIGHT, SPEED, UTC, POSITION).
 * @retval: Module status indicating the success or failure of the operation.
 */

Module_Status SampleToPort(uint8_t dstModule, uint8_t dstPort, All_Data dataFunction) {
    Module_Status Status = H08R6_OK;
    static uint8_t Temp[32] = {0}; /* Buffer for data transmission */

    /* Check if the port and module ID are valid */
    if ((dstPort == 0) && (dstModule == myID)) {
        return H08R6_ERR_WRONGPARAMS;
    }

    /* Sample and export data based on function type */
    switch (dataFunction) {
        case DISTANCE:
            if (SampleDistance(Distance) != H08R6_OK) {
                return H08R6_ERROR;
            }

            if (dstModule == myID) {
            	memcpy(Temp,Distance, sizeof(Distance));
                writePxITMutex(dstPort, (char*)&Temp[0], sizeof(Distance), 10);
            } else {
                /* LSB first */
                MessageParams[0] = FMT_INT16;                                    /* Data format: float */
                MessageParams[1] = (H08R6_OK == Status) ? BOS_OK : BOS_ERROR;   /* Operation status */
                MessageParams[2] = 16;                                           /* Number of elements (Height) */
                MessageParams[3] = (uint8_t)(CODE_H08R7_SAMPLE_DISTANCE >> 0);      /* Command code LSB */
                MessageParams[4] = (uint8_t)(CODE_H08R7_SAMPLE_DISTANCE >> 8);      /* Command code MSB */
                memcpy(&MessageParams[5],Distance, sizeof(Distance));

                SendMessageToModule(dstModule, CODE_READ_RESPONSE, sizeof(Distance) + 5);
            }
            break;

        case AVERAGE:
            if (SampleDistanceAverage(&Average) != H08R6_OK) {
                return H08R6_ERROR;
            }

            if (dstModule == myID) {
                /* LSB first */
                Temp[0] = (uint8_t) Average;         /* SpeedInch byte 0 */
                Temp[1] = (uint8_t)(Average >> 8);  /* SpeedInch byte 1 */

                writePxITMutex(dstPort, (char*)&Temp[0], 2 * sizeof(uint8_t), 10);
            } else {
                /* LSB first */
                MessageParams[0] = FMT_INT16;                                    /* Data format: float */
                MessageParams[1] = (H08R6_OK == Status) ? BOS_OK : BOS_ERROR;   /* Operation status */
                MessageParams[2] = 1;                                           /* Number of elements (SpeedInch, SpeedKm) */
                MessageParams[3] = (uint8_t)(CODE_H08R7_SAMPLE_DISTANCE_AVRG >> 0);       /* Command code LSB */
                MessageParams[4] = (uint8_t)(CODE_H08R7_SAMPLE_DISTANCE_AVRG >> 8);       /* Command code MSB */
                MessageParams[5] = (uint8_t)(*(uint32_t*)&Average);          /* SpeedInch byte 0 */
                MessageParams[6] = (uint8_t)((*(uint32_t*)&Average) >> 8);   /* SpeedInch byte 1 */

                SendMessageToModule(dstModule, CODE_READ_RESPONSE, 7 * sizeof(uint8_t));
            }
            break;

        case MOTION:
            if (MotionIndicator(Motion) != H08R6_OK) {
                return H08R6_ERROR;
            }

            if (dstModule == myID) {
            	memcpy(Temp,Motion, sizeof(Motion));
                writePxITMutex(dstPort, (char*)&Temp[0], sizeof(Motion), 10);
            } else {
                /* LSB first */
                MessageParams[0] = FMT_INT16;                                    /* Data format: int32 */
                MessageParams[1] = (H08R6_OK == Status) ? BOS_OK : BOS_ERROR;   /* Operation status */
                MessageParams[2] = 16;                                           /* Number of elements (Hours, Minutes, Seconds) */
                MessageParams[3] = (uint8_t)(CODE_H08R7_MOTION_INDICATOR >> 0);         /* Command code LSB */
                MessageParams[4] = (uint8_t)(CODE_H08R7_MOTION_INDICATOR >> 8);         /* Command code MSB */
                memcpy(&MessageParams[5],Motion, sizeof(Motion));

                SendMessageToModule(dstModule, CODE_READ_RESPONSE, sizeof(Motion) + 5);
            }
            break;

        case NUM_OF_TARGET:
            if (NumberOfTargets(NumOfTargets) != H08R6_OK) {
                return H08R6_ERROR;
            }

            if (dstModule == myID) {
            	memcpy(Temp,NumOfTargets, sizeof(NumOfTargets));
                writePxITMutex(dstPort, (char*)&Temp[0], sizeof(NumOfTargets), 10);
            } else {
                /* LSB first */
                MessageParams[0] = FMT_INT16;                                    /* Data format: float */
                MessageParams[1] = (H08R6_OK == Status) ? BOS_OK : BOS_ERROR;   /* Operation status */
                MessageParams[2] = 16;                                           /* Number of elements (LongDegree, LatDegree, LongIndicator, LatIndicator) */
                MessageParams[3] = (uint8_t)(CODE_H08R7_NUM_OF_TARGETS >> 0);    /* Command code LSB */
                MessageParams[4] = (uint8_t)(CODE_H08R7_NUM_OF_TARGETS >> 8);    /* Command code MSB */
                memcpy(&MessageParams[5],NumOfTargets, sizeof(NumOfTargets));

                SendMessageToModule(dstModule, CODE_READ_RESPONSE, sizeof(NumOfTargets) + 5);
            }
            break;

        default:
            return H08R6_ERR_WRONGPARAMS;
    }

    /* Clear the temp buffer */
    memset(Temp, 0, sizeof(Temp));

    return Status;
}

/***************************************************************************/
/* Streams a single sensor data sample to the terminal.
 * dstPort: Port number to stream data to.
 * dataFunction: Function to sample data (e.g., ACC, GYRO, MAG, TEMP).
 */
Module_Status SampleToTerminal(uint8_t dstPort,All_Data dataFunction)
{
	Module_Status Status =H08R6_OK; /* Initialize operation status as success */
	int8_t *PcOutputString = NULL; /* Pointer to CLI output buffer */
	uint32_t Period =0u; /* Calculated period for the operation */
	char CString[100] ={0}; /* Buffer for formatted output string */
//	int16_t * Distance, Average, * Motion, * NumOfTargets;

	/* Process data based on the requested sensor function */
	switch(dataFunction){
		case DISTANCE:
			/* Get the CLI output buffer for writing */
			PcOutputString =FreeRTOS_CLIGetOutputBuffer();
			/* Sample accelerometer data in G units */
			if(SampleDistance(Distance) != H08R6_OK){
				return H08R6_ERROR; /* Return error if sampling fails */
			}
			/* Format accelerometer data into a string */
			for(int sample = 0 ; sample < 16 ; sample++)
			{
				snprintf(CString,50,"Distance[zone %d] : %d\r\n",sample+1, Distance[sample]);
				/* Send the formatted string to the specified port */
				writePxMutex(dstPort,(char* )CString,strlen((char* )CString),cmd500ms,HAL_MAX_DELAY);
				_DELAY_MS(5);
			}

			break;

		case AVERAGE:
			/* Get the CLI output buffer for writing */
			PcOutputString =FreeRTOS_CLIGetOutputBuffer();
			/* Sample gyroscope data in degrees per second */
			if(SampleDistanceAverage(&Average) != H08R6_OK){
				return H08R6_ERROR; /* Return error if sampling fails */
			}
			/* Format gyroscope data into a string */
			snprintf(CString,50,"Average : %d\r\n",Average);
			/* Send the formatted string to the specified port */
			writePxMutex(dstPort,(char* )CString,strlen((char* )CString),cmd500ms,HAL_MAX_DELAY);
			break;

		case MOTION:
			/* Get the CLI output buffer for writing */
			PcOutputString =FreeRTOS_CLIGetOutputBuffer();
			/* Sample magnetometer data in milliGauss */
			if(MotionIndicator(Motion) != H08R6_OK){
				return H08R6_ERROR; /* Return error if sampling fails */
			}
			/* Format magnetometer data into a string */
			for(int sample = 0 ; sample < 16 ; sample++)
			{
				snprintf(CString,50,"Motion[zone %d] : %d\r\n",sample+1, Motion[sample]);
				/* Send the formatted string to the specified port */
				writePxMutex(dstPort,(char* )CString,strlen((char* )CString),cmd500ms,HAL_MAX_DELAY);
				_DELAY_MS(5);
			}
			/* Send the formatted string to the specified port */
			writePxMutex(dstPort,(char* )CString,strlen((char* )CString),cmd500ms,HAL_MAX_DELAY);
			break;

		case NUM_OF_TARGET:
			/* Get the CLI output buffer for writing */
			PcOutputString =FreeRTOS_CLIGetOutputBuffer();
			/* Sample temperature data in Celsius */
			if(NumberOfTargets(NumOfTargets) != H08R6_OK){
				return H08R6_ERROR; /* Return error if sampling fails */
			}
			/* Format temperature data into a string */
			for(int sample = 0 ; sample < 16 ; sample++)
			{
				snprintf(CString,50,"Num Of Target[zone %d] : %d\r\n",sample+1, NumOfTargets[sample]);
				/* Send the formatted string to the specified port */
				writePxMutex(dstPort,(char* )CString,strlen((char* )CString),cmd500ms,HAL_MAX_DELAY);
				_DELAY_MS(5);
			}
			break;

		default:
			/* Return error for invalid sensor function */
			return H08R6_ERR_WRONGPARAMS;
	}

	/* Return final status indicating success or prior error */
	return Status;
}

/***************************************************************************/
/*
 * brief: Streams data to the specified port and module with a given number of samples.
 * param targetModule: The target module to which data will be streamed.
 * param portNumber: The port number on the module.
 * param portFunction: Type of data that will be streamed (ACC, GYRO, MAG, or TEMP).
 * param numOfSamples: The number of samples to stream.
 * param streamTimeout: The interval (in milliseconds) between successive data transmissions.
 * retval: of type Module_Status indicating the success or failure of the operation.
 */

Module_Status StreamToPort(uint8_t dstModule,uint8_t dstPort,All_Data dataFunction,uint32_t numOfSamples,uint32_t streamTimeout)
{
	Module_Status Status =H08R6_OK;
	uint32_t SamplePeriod =0u;

	/* Check timer handle and timeout validity */
	if((NULL == xTimerStream) || (0 == streamTimeout) || (0 == numOfSamples))
		return H08R6_ERROR; /* Assuming H0BR4_ERROR is defined in Module_Status */

	/* Set streaming parameters */
	StreamMode = STREAM_MODE_TO_PORT;
	PortModule =dstModule;
	PortNumber =dstPort;
	PortFunction =dataFunction;
	PortNumOfSamples =numOfSamples;

	/* Calculate the period from timeout and number of samples */
	SamplePeriod =streamTimeout / numOfSamples;

	/* Stop (Reset) the TimerStream if it's already running */
	if(xTimerIsTimerActive(xTimerStream)){
		if(pdFAIL == xTimerStop(xTimerStream,100))
			return H08R6_ERROR;
	}

	/* Start the stream timer */
	if(pdFAIL == xTimerStart(xTimerStream,100))
		return H08R6_ERROR;

	/* Update timer timeout - This also restarts the timer */
	if(pdFAIL == xTimerChangePeriod(xTimerStream,SamplePeriod,100))
		return H08R6_ERROR;

	return Status;
}

/***************************************************************************/
/*
 * brief: Streams data to the specified terminal port with a given number of samples.
 * param targetPort: The port number on the terminal.
 * param dataFunction: Type of data that will be streamed (ACC, GYRO, MAG, or TEMP).
 * param numOfSamples: The number of samples to stream.
 * param streamTimeout: The interval (in milliseconds) between successive data transmissions.
 * retval: of type Module_Status indicating the success or failure of the operation.
 */

Module_Status StreamToTerminal(uint8_t dstPort,All_Data dataFunction,uint32_t numOfSamples,uint32_t streamTimeout)
{
	Module_Status Status =H08R6_OK;
	uint32_t SamplePeriod =0u;
	/* Check timer handle and timeout validity */
	if((NULL == xTimerStream) || (0 == streamTimeout) || (0 == numOfSamples))
		return H08R6_ERROR; /* Assuming H0BR4_ERROR is defined in Module_Status */

	/* Set streaming parameters */
	StreamMode = STREAM_MODE_TO_TERMINAL;
	TerminalPort =dstPort;
	TerminalFunction =dataFunction;
	TerminalNumOfSamples =numOfSamples;

	/* Calculate the period from timeout and number of samples */
	SamplePeriod =streamTimeout / numOfSamples;

	/* Stop (Reset) the TimerStream if it's already running */
	if(xTimerIsTimerActive(xTimerStream)){
		if(pdFAIL == xTimerStop(xTimerStream,100))
			return H08R6_ERROR;
	}

	/* Start the stream timer */
	if(pdFAIL == xTimerStart(xTimerStream,100))
		return H08R6_ERROR;

	/* Update timer timeout - This also restarts the timer */
	if(pdFAIL == xTimerChangePeriod(xTimerStream,SamplePeriod,100))
		return H08R6_ERROR;

	return Status;
}

/***************************************************************************/
/*
 * @brief: Streams data to a buffer.
 * @param buffer: Pointer to the buffer where data will be stored.
 * @param function: Function to sample data (e.g., ACC, GYRO, MAG, TEMP).
 * @param Numofsamples: Number of samples to take.
 * @param timeout: Timeout period for the operation.
 * @retval: Module status indicating success or error.
 */

Module_Status StreamToBuffer(int16_t *buffer,All_Data function, uint32_t Numofsamples, uint32_t timeout)
{
	switch(function){
		case DISTANCE:
			return StreamToBuf(buffer,Numofsamples,timeout,SampleDistanceBuf);
			break;
		case AVERAGE:
			return StreamToBuf(buffer,Numofsamples,timeout,SampleDistanceAverageBuf);
			break;
		case MOTION:
			return StreamToBuf(buffer,Numofsamples,timeout,MotionIndicatorBuf);
			break;
		case NUM_OF_TARGET:
			return StreamToBuf(buffer,Numofsamples,timeout,NumberOfTargetsBuf);
			break;
		default:
			break;
	}
}

/***************************************************************************/
/* Callback function triggered by a timer to manage data streaming.
 * xTimerStream: Handle of the timer that triggered the callback.
 */
void StreamTimeCallback(TimerHandle_t xTimerStream)
{
	/* Increment sample counter */
	++SampleCount;

	/* Stream mode to port: Send samples to port */
	if(STREAM_MODE_TO_PORT == StreamMode){
		if((SampleCount <= PortNumOfSamples) || (0 == PortNumOfSamples)){
			SampleToPort(PortModule,PortNumber,PortFunction);
		}
		else{
			SampleCount =0;
			xTimerStop(xTimerStream,0);
			endStreamFlag = 1;
		}
	}
	/* Stream mode to terminal: Export to terminal */
	else if(STREAM_MODE_TO_TERMINAL == StreamMode){
		if((SampleCount <= TerminalNumOfSamples) || (0 == TerminalNumOfSamples)){
			SampleToTerminal(TerminalPort,TerminalFunction);
		}
		else{
			SampleCount =0;
			xTimerStop(xTimerStream,0);
			endStreamFlag = 1;
		}
	}
}

/***************************************************************************/
static Module_Status StreamToCLI(uint32_t Numofsamples, uint32_t timeout, SampleToString function) {
	Module_Status status = H08R6_OK;
	int8_t *pcOutputString = NULL;
	uint32_t period = timeout / Numofsamples;

	if (period < MIN_MEMS_PERIOD_MS)
		return H08R6_ERR_WRONGPARAMS;

	// TODO: Check if CLI is enable or not
	for (uint8_t chr = 0; chr < MSG_RX_BUF_SIZE; chr++) {
		if (UARTRxBuf [pcPort - 1] [chr] == '\r') {
			UARTRxBuf [pcPort - 1] [chr] = 0;
		}
	}
	if (1 == flag) {
		flag = 0;
		static char *pcOKMessage = (int8_t*) "Stop stream !\n\r";
		writePxITMutex(pcPort, pcOKMessage, strlen(pcOKMessage), 10);
		return status;
	}
	if (period > timeout)
		timeout = period;

	long numTimes = timeout / period;
	stopstream = false;

	while ((numTimes-- > 0) || (timeout >= MAX_MEMS_TIMEOUT_MS)) {
		pcOutputString = FreeRTOS_CLIGetOutputBuffer();
		function((char*) pcOutputString, 100);

		writePxMutex(pcPort, (char*) pcOutputString, strlen((char*) pcOutputString), cmd500ms, HAL_MAX_DELAY);
		if (PollingSleepCLISafe(period, Numofsamples) != H08R6_OK)
			break;
	}

	memset((char*) pcOutputString, 0, configCOMMAND_INT_MAX_OUTPUT_SIZE);
	sprintf((char*) pcOutputString, "\r\n");

	return status;
}

/***************************************************************************/
/* Streams sensor data to a buffer.
 * buffer: Pointer to the buffer where data will be stored.
 * Numofsamples: Number of samples to take.
 * timeout: Timeout period for the operation.
 * function: Function pointer to the sampling function (e.g., SampleAccBuf, SampleGyroBuf).
 */
static Module_Status StreamToBuf(int16_t *buffer,uint32_t Numofsamples,uint32_t timeout,SampleToBuffer function){
	Module_Status status =H08R6_OK;
	uint16_t StreamIndex =0;
	uint32_t period =timeout / Numofsamples;
	int16_t sample[16];
	/* Check if the calculated period is valid */
	if(period < MIN_PERIOD_MS)
		return H08R6_ERR_WRONGPARAMS;

	stopstream = false;

	/* Stream data to buffer */
	while((Numofsamples-- > 0) || (timeout >= MAX_TIMEOUT_MS)){

		function(sample);
		/* Delay for the specified period */
		vTaskDelay(pdMS_TO_TICKS(period));

		if(function == SampleDistanceAverageBuf)
		{
			buffer[StreamIndex] =sample[0];
			StreamIndex++;
		}
		else
		{
			for(int index = 0 ; index < 16 ; index++)
			{
				buffer[StreamIndex] =sample[index];
				StreamIndex++;
			}
		}

		/* Check if streaming should be stopped */
		if(stopstream){
			status =H08R6_ERR_TERMINATED;
			break;
		}
	}

	return status;
}

/***************************************************************************/
/* Samples accelerometer data into a buffer.
 * buffer: Pointer to the buffer where accelerometer data will be stored.
 */
void SampleDistanceBuf(int16_t *buffer){
	SampleDistance(buffer);
}

/***************************************************************************/
/* Samples gyroscope data into a buffer.
 * buffer: Pointer to the buffer where gyroscope data will be stored.
 */
void SampleDistanceAverageBuf(int16_t *buffer){
	SampleDistanceAverage(buffer);
}

/***************************************************************************/
/* Samples magnetometer data into a buffer.
 * buffer: Pointer to the buffer where magnetometer data will be stored.
 */
void MotionIndicatorBuf(int16_t *buffer){
	MotionIndicator(buffer);
}

/***************************************************************************/
/* Samples temperature data into a buffer.
 * buffer: Pointer to the buffer where temperature data will be stored.
 */
void NumberOfTargetsBuf(int16_t *buffer){
	NumberOfTargets(buffer);
}


/***************************************************************************/
void SampleDistanceToString(char *cstring, size_t maxLen) {
	int16_t * Distance;

	SampleDistance(Distance);

	for(int sample = 0 ; sample <= 16 ; sample++)
	{
		snprintf(cstring, maxLen, "TOF: distance: %d\r\n",Distance[sample]);
	}
}

/***************************************************************************/
void SampleDistanceAverageToString(char *cstring, size_t maxLen) {
	int16_t Average;

	SampleDistanceAverage(&Average);
	snprintf(cstring, maxLen, "TOF: average : %d\r\n",Average);

}

/***************************************************************************/
void MotionIndicatorToString(char *cstring, size_t maxLen) {
	int16_t * Indicator;

	MotionIndicator(Indicator);
	for(int sample = 0 ; sample <= 16 ; sample++)
	{
		snprintf(cstring, maxLen, "TOF: indicator: %d\r\n", Indicator[sample]);
	}

}

/***************************************************************************/
void NumberOfTargetsToString(char *cstring, size_t maxLen) {
	int16_t * NumOfTargets;

	NumberOfTargets(NumOfTargets);
	for(int sample = 0 ; sample <= 16 ; sample++)
	{
		snprintf(cstring, maxLen, "TOF: numOfTargets: %d\r\n", NumOfTargets[sample]);
	}

}

/***************************************************************************/
void StopStream(void) {
	stopstream = true;
}

/***************************************************************************/
/***************************** General Functions ***************************/
/***************************************************************************/

Module_Status SampleDistance(int16_t *distance) {
	Module_Status status = H08R6_OK;

	if ((status = VL53L8CX_SampleDistance(distance)) != H08R6_OK)
		return status = H08R6_ERROR;

	return status;
}

/***************************************************************************/
Module_Status SampleDistanceAverage(int16_t *average) {
	Module_Status status = H08R6_OK;

	if ((status = VL53L8CX_SampleDistanceAverage(average)) != H08R6_OK)
		return status = H08R6_ERROR;

	return status;
}

/***************************************************************************/
Module_Status MotionIndicator(int16_t *indicator) {
	Module_Status status = H08R6_OK;

	if ((status = VL53L8CX_MotionIndicator(indicator)) != H08R6_OK)
		return status = H08R6_ERROR;

	return status;
}

/***************************************************************************/
Module_Status NumberOfTargets(int16_t *numOfTargets) {
	Module_Status status = H08R6_OK;

	if ((status = VL53L8CX_NumberofTargets(numOfTargets)) != H08R6_OK)
		return status = H08R6_ERROR;

	return status;
}

/***************************************************************************/
/********************************* Commands ********************************/
/***************************************************************************/
static portBASE_TYPE SampleTOFCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
	const char *const distanceCmdName = "distance";
	const char *const averageCmdName = "average";
	const char *const motionName = "motion";
	const char *const numOfTargetCmdName = "numoftarget";

	const char *pfuncName = NULL;
	const char *pPortCliStr = NULL;
	const char *pPortStr = NULL;
	const char *pModStr = NULL;

	portBASE_TYPE funcNameLen = 0;
	portBASE_TYPE portCliStrLen = 0;
	portBASE_TYPE portStrLen = 0;
	portBASE_TYPE modStrLen = 0;

	uint8_t port;
	bool portOrCLI = true; // Port Mode => false and CLI Mode => true
	/* Make sure we return something */
	*pcWriteBuffer = '\0';

	pfuncName = (const char*) FreeRTOS_CLIGetParameter(pcCommandString, 1, &funcNameLen);

	if (pfuncName == NULL) {
		snprintf((char*) pcWriteBuffer, xWriteBufferLen, "Invalid Arguments\r\n");
		return pdFALSE;
	}
	pPortCliStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 2, &portCliStrLen);
	pPortStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 3, &portStrLen);
	pModStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 4, &modStrLen);

	portOrCLI = atoi(pPortCliStr);
	port = atoi(pPortStr);
	dstModule = atoi(pModStr);


	if (!strncmp(pfuncName, distanceCmdName, strlen(distanceCmdName))) {
		dataFunction = DISTANCE;
	} else if (!strncmp(pfuncName, averageCmdName, strlen(averageCmdName))) {
		dataFunction = AVERAGE;
	} else if (!strncmp(pfuncName, motionName, strlen(motionName))) {
		dataFunction = MOTION;
	} else if (!strncmp(pfuncName, numOfTargetCmdName, strlen(numOfTargetCmdName))) {
		dataFunction = NUM_OF_TARGET;
	} else {
		snprintf((char*) pcWriteBuffer, xWriteBufferLen, "Invalid Arguments\r\n");
		return pdFALSE;
	}

	if (portOrCLI) {
		dstPort = pcPort;
		streamFlag = 1;
	} else {
		dstPort = port;
		streamFlag = 2;
	}
	while(streamFlag != 0);

	return pdFALSE;
}

static portBASE_TYPE StreamTOFCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
	const char *const distanceCmdName = "distance";
	const char *const averageCmdName = "average";
	const char *const motionName = "motion";
	const char *const numOfTargetCmdName = "numoftarget";

	uint32_t Numofsamples = 0;
	uint32_t timeout = 0;
	uint8_t port = 0;
	uint8_t module = 0;

	bool portOrCLI = true; // Port Mode => false and CLI Mode => true

	const char *pSensName = NULL;
	portBASE_TYPE sensNameLen = 0;

	// Make sure we return something
	*pcWriteBuffer = '\0';

	if (!StreamCommandParser(pcCommandString, &pSensName, &sensNameLen, &portOrCLI, &Numofsamples, &timeout, &port,
			&module)) {
		snprintf((char*) pcWriteBuffer, xWriteBufferLen, "Invalid Arguments\r\n");
		return pdFALSE;
	}



	dstModule = module;
	numOfSamples = Numofsamples;
	streamTimeout = timeout;

	if (!strncmp(pSensName, distanceCmdName, strlen(distanceCmdName))) {
		dataFunction = DISTANCE;
	} else if (!strncmp(pSensName, averageCmdName, strlen(averageCmdName))) {
		dataFunction = AVERAGE;
	} else if (!strncmp(pSensName, motionName, strlen(motionName))) {
		dataFunction = MOTION;
	} else if (!strncmp(pSensName, numOfTargetCmdName, strlen(numOfTargetCmdName))) {
		dataFunction = NUM_OF_TARGET;
	} else {
		snprintf((char*) pcWriteBuffer, xWriteBufferLen, "Invalid Arguments\r\n");
		return pdFALSE;
	}

	if (portOrCLI) {
		dstPort = pcPort;
		streamFlag = 3;
	} else {
		dstPort = port;
		streamFlag = 4;
	}
	while(endStreamFlag == 0);
	endStreamFlag = 0;

	return pdFALSE;
}
/***************************************************************************/
static bool StreamCommandParser(const int8_t *pcCommandString, const char **ppSensName, portBASE_TYPE *pSensNameLen,
														bool *pPortOrCLI, uint32_t *pNumOfSamples, uint32_t *pTimeout, uint8_t *pPort, uint8_t *pModule)
{
	const char *pNumOfSamplesStr = NULL;
	const char *pTimeoutMSStr = NULL;

	portBASE_TYPE numOfSamplesStrLen = 0;
	portBASE_TYPE timeoutStrLen = 0;

	const char *pPortCliStr = NULL;
	const char *pPortStr = NULL;
	const char *pModStr = NULL;

	portBASE_TYPE portCliStrLen = 0;
	portBASE_TYPE portStrLen = 0;
	portBASE_TYPE modStrLen = 0;

	*ppSensName = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, pSensNameLen);
	pNumOfSamplesStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 2, &numOfSamplesStrLen);
	pTimeoutMSStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 3, &timeoutStrLen);
	pPortCliStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 4, &portCliStrLen);

	// At least 3 Parameters are required!
	if ((*ppSensName == NULL) || (pNumOfSamplesStr == NULL) || (pTimeoutMSStr == NULL))
		return false;

	// TODO: Check if Period and Timeout are integers or not!
	*pNumOfSamples = atoi(pNumOfSamplesStr);
	*pTimeout = atoi(pTimeoutMSStr);
	*pPortOrCLI = atoi(pPortCliStr);

	pPortStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 5, &portStrLen);
	pModStr = (const char *)FreeRTOS_CLIGetParameter(pcCommandString, 6, &modStrLen);

	if ((pModStr == NULL) && (pPortStr == NULL))
		return true;
	if ((pModStr == NULL) || (pPortStr == NULL))	// If user has provided 4 Arguments.
		return false;

	*pPort = atoi(pPortStr);
	*pModule = atoi(pModStr);

	return true;
}

/***************************************************************************/
/***************** (C) COPYRIGHT HEXABITZ ***** END OF FILE ****************/
