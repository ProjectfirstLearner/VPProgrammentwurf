/******************************************************************************
 * @file main.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Main file for the VP Template Authenticator project
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"
#include "System.h"

#include "HardwareConfig.h"

#include "Util/Global.h"
#include "Util/Log/printf.h"
#include "Util/Log/LogOutput.h"

#include "UARTModule.h"
#include "LEDModule.h"
#include "DisplayModule.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/

#define UART_A_RX_FAILURE_MS			15000
#define KEY_STAGE1_TIMEOUT_MS			10000
#define KEY_STAGE2_TIMEOUT_MS			30000
#define KEY_FAILURE_TIMEOUT_MS			45000

#define TIMER_START_VALUE				0
#define EMPTY_UART_VALUE				0

#define INITIALIZE_FALSE				0
#define INITIALIZE_TRUE					1

#define UART_RX_ONE_BYTE				1
#define UART_TRIGGER_CHAR				'A'
#define UART_KEY_TERMINATOR				'\n'
#define UART_KEY_MAX_LENGTH				8u

#define LED_FLASH_PERIOD_MS				250

/***** PRIVATE TYPES *********************************************************/

// State Machine States
typedef enum
{
	STATE_BOOTUP = 0, //giving identification 0, 1-3 will be added automatically to the other states
	STATE_FAILURE,
	STATE_PRE_APP,
	STATE_START_APP
} AuthState_t;

// PreApp States
typedef enum
{
	PRE_APP_WAIT_FOR_TRIGGER = 0,
	PRE_APP_RECEIVE_KEY,
	PRE_APP_KEY_READY
} PreAppSubState_t;

// Receiving Byte from Python
typedef struct
{
	uint8_t buffer[UART_KEY_MAX_LENGTH];
	uint8_t length;
	uint8_t complete;
	uint8_t overflow;
	uint32_t startTick;
	uint8_t initialized;
} KeyRxContext_t;

/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals();
static void HandleBootupState();
static void HandlePreAppState();
static void HandleAppStartState();
static void HandleFailureState();

static uint32_t DecryptAuthSection(uint8_t* pData, uint32_t dataLength,
								   const uint8_t* pKey, uint32_t keyLength);
static uint32_t ProcessAuthDecryption(uint8_t* pData, uint32_t dataLength,
								      const uint8_t* pKey, uint32_t keyLength);

// Key Functions
static void KeyRxInit(KeyRxContext_t* pContext);
static void KeyRxProcess(KeyRxContext_t* pContext);
static uint32_t KeyRxGetElapsedTimeMs(const KeyRxContext_t* pContext);
static void UpdateKeyTimeoutIndication(uint32_t elapsedTimeMs);

/***** PRIVATE VARIABLES *****************************************************/

static AuthState_t gAuthState = STATE_BOOTUP; //global within scope

static uint32_t gPreAppTickStart = TIMER_START_VALUE;
static uint8_t gPreAppInitialized = INITIALIZE_FALSE;

static PreAppSubState_t gPreAppSubState = PRE_APP_WAIT_FOR_TRIGGER;
static KeyRxContext_t gKeyRxContext = {0};

/***** PUBLIC FUNCTIONS ******************************************************/
/**
 * @brief Main function of Authenticator
 */
int main(void)
{
    while (1)
    {
    	//implementing state machine using switch case
    	switch (gAuthState){

    		case STATE_BOOTUP:
    			//all init functions
    			HandleBootupState();
    			break;
    		case STATE_PRE_APP:
    			//allpreStarting functions
    			HandlePreAppState();
    			break;
    		case STATE_START_APP:
    			//all functions to start app
    			HandleAppStartState();
    			break;
    		case STATE_FAILURE:
    			//functions to handle Failure
    			HandleFailureState();
    			break;
    	}
    }
}

/***** PRIVATE FUNCTIONS *****************************************************/
/**
 * @brief Initializes the used peripherals like GPIO,
 * ADC, DMA and Timer Interrupts
 *
 * @return Returns ERROR_OK if no error occurred
 */
static int32_t initializePeripherals()
{
    // Initialize UART used for Debug-Outputs
    uartInitialize(115200);

    // Initialize GPIOs for LED and 7-Segment output
	ledInitialize();

    return ERROR_OK;
}

/**
 * @brief Handles Bootup of authenticator
 */
static void HandleBootupState()
{
	HAL_StatusTypeDef halStatus;

	halStatus = HAL_Init();
	// If HAL initialization was not Successful: State -> FAILURE
	if(halStatus != HAL_OK) {
		gAuthState = STATE_FAILURE;
		return;
	}

	// Set correct Clock Timing
	SystemClock_Config();

	// If Peripheral initialization was not Successful: State -> FAILURE
	if(initializePeripherals() != ERROR_OK) {
		gAuthState = STATE_FAILURE;
		return;
	}

	// Turning on LED D0 and Resetting D1 & D2
	ledSetLED(LED0, GPIO_PIN_SET);
	ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED2, GPIO_PIN_RESET);

	gAuthState = STATE_PRE_APP;
}

/**
 * @brief Handles Pre-Application of authenticator
 */
static void HandlePreAppState()
{
	// Set the Timer for Timeout to 0
	uint32_t timeElapsed = TIMER_START_VALUE;
	int8_t hasChar = EMPTY_UART_VALUE;
	uint8_t ch = EMPTY_UART_VALUE;
	uint32_t receiveStatus = UART_ERR_OK;

	// Initialize Tick for Timer only once
	if (gPreAppInitialized == INITIALIZE_FALSE)
	{
		gPreAppTickStart = HAL_GetTick();
		gPreAppInitialized = INITIALIZE_TRUE;
		gPreAppSubState = PRE_APP_WAIT_FOR_TRIGGER;
	}

	switch(gPreAppSubState)
	{
		case PRE_APP_WAIT_FOR_TRIGGER:
		{
			// Switching to Failure-State if the 15s timeout is reached
			timeElapsed = HAL_GetTick() - gPreAppTickStart;
			if(timeElapsed >= UART_A_RX_FAILURE_MS)
			{
			    gAuthState = STATE_FAILURE;
			    return;
			}

			// Reading UART Input
			uartHasData(&hasChar);

		    if (hasChar != EMPTY_UART_VALUE)
		    {
		        receiveStatus = uartReceiveData(&ch, UART_RX_ONE_BYTE);

		        if ((receiveStatus == UART_ERR_OK) && (ch == UART_TRIGGER_CHAR))
		        {
		            KeyRxInit(&gKeyRxContext);
		            gPreAppSubState = PRE_APP_RECEIVE_KEY;
		            return;
		        }
		    }
		    break;
		}
		case PRE_APP_RECEIVE_KEY:
		{
			KeyRxProcess(&gKeyRxContext);
			timeElapsed = KeyRxGetElapsedTimeMs(&gKeyRxContext);

			UpdateKeyTimeoutIndication(timeElapsed);

			if(gKeyRxContext.overflow != 0)
			{
				gAuthState = STATE_FAILURE;
				return;
			}

			if(timeElapsed >= KEY_FAILURE_TIMEOUT_MS)
			{
				gAuthState = STATE_FAILURE;
				return;
			}

			if(gKeyRxContext.complete != 0)
			{
				if(gKeyRxContext.length == 0)
				{
					gAuthState = STATE_FAILURE;
					return;
				}

				gPreAppSubState = PRE_APP_KEY_READY;
				return;
			}
			break;
		}
		case PRE_APP_KEY_READY:
		{
			ledSetLED(LED3, GPIO_PIN_SET);

			// Hier .auth vom FLASH in RAM kopieren

			//gAuthState = STATE_START_APP;
			//return;
		}
	}
}

/**
 * @brief Function for Decrypting Auth Key
 */
static uint32_t DecryptAuthSection(uint8_t* pData, uint32_t dataLength, const uint8_t* pKey, uint32_t keyLength)
{
	uint32_t dataIndex	= 0;
	uint32_t keyIndex	= 0;

	// Check for Valid Data and Null-Pointer
	if((pData == NULL) || (pKey == NULL) || (dataLength == 0) || (keyLength == 0)) return ERROR_GENERAL;

	for(dataIndex = 0; dataIndex < dataLength; ++dataIndex) {
		pData[dataIndex] = pData[dataIndex] ^ pKey[keyIndex];

		keyIndex++;
		if(keyIndex >= keyLength) keyIndex = 0;
	}

	return ERROR_OK;
}

/**
 * @brief Function for Decrypting Auth Key
 */
static uint32_t ProcessAuthDecryption(uint8_t* pData, uint32_t dataLength, const uint8_t* pKey, uint32_t keyLength)
{
	if ((pData == NULL) || (pKey == NULL)) return ERROR_GENERAL;

#ifdef ENABLE_ENCRYPTION
	return DecryptAuthSection(pData, dataLength, pKey, keyLength);
#else
	(void)dataLength;
	(void)keyLength;
	return ERROR_OK;
#endif
}

/**
 * @brief Function for Handeling Key Receiving
 */
static void KeyRxInit(KeyRxContext_t* pContext)
{
	uint32_t index = 0;

	if(pContext == NULL) return;

	for(index = 0; index < UART_KEY_MAX_LENGTH; index++) {
		pContext->buffer[index] = 0;
	}

	pContext->length		= 0;
	pContext->complete		= 0;
	pContext->overflow		= 0;
	pContext->startTick		= HAL_GetTick();
	pContext->initialized	= INITIALIZE_TRUE;
}

/**
 * @brief Function for Processing Key
 */
static void KeyRxProcess(KeyRxContext_t* pContext)
{
	int8_t hasChar = EMPTY_UART_VALUE;
	uint8_t ch = EMPTY_UART_VALUE;
	uint32_t receiveStatus = UART_ERR_OK;

	if((pContext == NULL) || (pContext->initialized == INITIALIZE_FALSE) || (pContext->complete != 0)) return;

	while(1)
	{
		hasChar = EMPTY_UART_VALUE;
		uartHasData(&hasChar);

		if(hasChar == EMPTY_UART_VALUE) break;

		receiveStatus = uartReceiveData(&ch, UART_RX_ONE_BYTE);

		if(receiveStatus != UART_ERR_OK) break;

		if(ch == (uint8_t)UART_KEY_TERMINATOR)
		{
			pContext->complete = 1;
			break;
		}

		if(pContext->length < UART_KEY_MAX_LENGTH)
		{
			pContext->buffer[pContext->length] = ch;
			pContext->length++;
		}
		else
		{
			pContext->overflow = 1;
			break;
		}
	}
}

/**
 * @brief Function for getting Elapsed Time
 */
static uint32_t KeyRxGetElapsedTimeMs(const KeyRxContext_t* pContext)
{
	if((pContext == NULL) || (pContext->initialized == INITIALIZE_FALSE)) return 0;

	return (HAL_GetTick() - pContext->startTick);
}

/**
 * @brief Function for updating Timeout Indicators
 */
static void UpdateKeyTimeoutIndication(uint32_t elapsedTimeMs)
{
	// Failure State after 45s
	if(elapsedTimeMs >= KEY_FAILURE_TIMEOUT_MS) gAuthState = STATE_FAILURE;

	// LED1 blinking after 30s
	else if(elapsedTimeMs >= KEY_STAGE2_TIMEOUT_MS)
	{
		if(((HAL_GetTick() / LED_FLASH_PERIOD_MS) % 2) == 0) ledSetLED(LED1, GPIO_PIN_SET);

		else ledSetLED(LED1, GPIO_PIN_RESET);
	}

	// Turning on LED1 after 10s
	else if(elapsedTimeMs >= KEY_STAGE1_TIMEOUT_MS) ledSetLED(LED1, GPIO_PIN_SET);
}

/**
 * @brief Handles Application Start of authenticator
 */
static void HandleAppStartState()
{
	//ledSetLED(LED0, GPIO_PIN_SET);
	//ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED2, GPIO_PIN_SET);
}

/**
 * @brief Handles Failure of authenticator
 */
static void HandleFailureState()
{
	ledSetLED(LED0, GPIO_PIN_RESET);
	ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED2, GPIO_PIN_RESET);
	ledSetLED(LED4, GPIO_PIN_SET);
}
