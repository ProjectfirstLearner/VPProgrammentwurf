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

#include "UARTModule.h"
#include "LEDModule.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/

// TIMEOUT CONSTANTS

#define UART_A_RX_FAILURE_MS			15000u
#define KEY_STAGE1_TIMEOUT_MS			10000u
#define KEY_STAGE2_TIMEOUT_MS			30000u
#define KEY_FAILURE_TIMEOUT_MS			45000u

// UART CONSTANTS

#define UART_RX_BYTE_COUNT				1u
#define UART_TRIGGER_CHAR				'A'
#define UART_KEY_TERMINATOR				'\n'
#define UART_KEY_MAX_LENGTH				8u

// INITIALIZATION FLAGS

#define PRE_APP_INITIALIZE_FLAG			1u
#define KEY_COMPLETE_FLAG				1u
#define KEY_OVERFLOW_FLAG				1u

// LED CONTROL

#define LED_FLASH_PERIOD_MS				250u
#define LED_FLASH_DIVIDER				2u

/***** PRIVATE TYPES *********************************************************/

// State-Machine States
typedef enum
{
	STATE_BOOTUP = 0,
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

/**
 * @brief Receives the decryption key via UART.
 */
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

static void EnterFailureState();
static void EnterPreAppState();

// State-Machine Functions
static void HandleBootupState();
static void HandlePreAppState();
static void HandleAppStartState();
static void HandleFailureState();

// Pre-Application Helper Functions
static void HandlePreAppWaitForTrigger();
static void HandlePreAppReceiveKey();
static void HandlePreAppKeyReady();

// Auth Section Functions
static int32_t CopyAuthSectionToRam();

// Encrypt / Decrypt Functions
static int32_t DecryptAuthSection(uint8_t* pData, uint32_t dataLength, const uint8_t* pKey, uint32_t keyLength);
static int32_t ProcessAuthDecryption(uint8_t* pData, uint32_t dataLength, const uint8_t* pKey, uint32_t keyLength);

// Key Functions
static void KeyRxInit(KeyRxContext_t* pContext);
static void KeyRxProcess(KeyRxContext_t* pContext);
static uint32_t KeyRxGetElapsedTimeMs(const KeyRxContext_t* pContext);
static void UpdateKeyTimeoutIndication(uint32_t elapsedTimeMs);

/***** PRIVATE VARIABLES *****************************************************/

static AuthState_t gAuthState = STATE_BOOTUP;

static uint32_t gPreAppTickStart = 0;
static uint8_t gPreAppInitialized = 0;

static PreAppSubState_t gPreAppSubState = PRE_APP_WAIT_FOR_TRIGGER;
static KeyRxContext_t gKeyRxContext = {0};

// Auth Section Variables
extern uint8_t _sauth_flash[];
extern uint8_t _eauth_flash[];
extern uint8_t _sauth_ram[];
extern uint8_t _eauth_ram[];
extern uint8_t _auth_size;

/***** PUBLIC FUNCTIONS ******************************************************/
/**
 * @brief Main function of Authenticator
 */
int main(void)
{
    while (1)
    {
    	// State Machine using Switch
    	switch (gAuthState){

    		case STATE_BOOTUP:
    			// Boot-up Function
    			HandleBootupState();
    			break;
    		case STATE_PRE_APP:
    			// Pre-Application Function
    			HandlePreAppState();
    			break;
    		case STATE_START_APP:
    			// Application Start Function
    			HandleAppStartState();
    			break;
    		case STATE_FAILURE:
    			// Failure Function
    			HandleFailureState();
    			break;
    	}
    }
}

/***** PRIVATE FUNCTIONS *****************************************************/
/**
 * @brief Initializes the used peripherals
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

static void EnterFailureState()
{
	gAuthState = STATE_FAILURE;
}

static void EnterPreAppState()
{
	gPreAppTickStart = HAL_GetTick();
	gPreAppInitialized = PRE_APP_INITIALIZE_FLAG;
	gPreAppSubState = PRE_APP_WAIT_FOR_TRIGGER;
	gAuthState = STATE_PRE_APP;
}

/**
 * @brief Handles Boot-up of authenticator
 */
static void HandleBootupState()
{
	// If HAL initialization was Successful continue otherwise State -> FAILURE
	if(HAL_Init() != HAL_OK)
	{
		EnterFailureState();
		return;
	}

	// Set correct Clock Timing
	SystemClock_Config();

	// If Peripheral initialization was Successful continue otherwise State -> FAILURE
	if(initializePeripherals() != ERROR_OK)
	{
		EnterFailureState();
		return;
	}

	// Turning on LED D0
	ledSetLED(LED0, GPIO_PIN_SET);

	// Setting the State to Pre-Application
	EnterPreAppState();
}

/**
 * @brief Handles Pre-Application of authenticator
 */
static void HandlePreAppState()
{
	// Initialize Tick for Timer only once
	if (gPreAppInitialized == 0)
	{
		gPreAppTickStart = HAL_GetTick();
		gPreAppInitialized = PRE_APP_INITIALIZE_FLAG;
		gPreAppSubState = PRE_APP_WAIT_FOR_TRIGGER;
	}

	switch(gPreAppSubState)
	{
		case PRE_APP_WAIT_FOR_TRIGGER:
			HandlePreAppWaitForTrigger();
			return;

		case PRE_APP_RECEIVE_KEY:
			HandlePreAppReceiveKey();
			break;

		case PRE_APP_KEY_READY:
			HandlePreAppKeyReady();
			break;
	}
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


static void HandlePreAppWaitForTrigger()
{
	// Set the Timer for Timeout to 0
	uint32_t timeElapsed = HAL_GetTick() - gPreAppTickStart;
	int8_t hasChar = 0;
	uint8_t ch = 0;
	uint32_t receiveStatus = UART_ERR_OK;

	// Switching to Failure-State if the 15s timeout is reached
	if(timeElapsed >= UART_A_RX_FAILURE_MS)
	{
		EnterFailureState();
		return;
	}

	// Reading UART Input
	uartHasData(&hasChar);

	if (hasChar == 0) return;

	receiveStatus = uartReceiveData(&ch, UART_RX_BYTE_COUNT);
	if ((receiveStatus == UART_ERR_OK) && (ch == (uint8_t)UART_TRIGGER_CHAR))
	{
		KeyRxInit(&gKeyRxContext);
		gPreAppSubState = PRE_APP_RECEIVE_KEY;
	}
}

static void HandlePreAppReceiveKey()
{
	uint32_t timeElapsed = 0u;

	KeyRxProcess(&gKeyRxContext);
	timeElapsed = KeyRxGetElapsedTimeMs(&gKeyRxContext);

	UpdateKeyTimeoutIndication(timeElapsed);

	if(gAuthState == STATE_FAILURE) return;

	if(gKeyRxContext.overflow != 0u)
	{
		EnterFailureState();
		return;
	}

	if(timeElapsed >= KEY_FAILURE_TIMEOUT_MS)
	{
		EnterFailureState();
		return;
	}

	if((gKeyRxContext.complete != 0u) && (gKeyRxContext.length > 0u))
	{
		gPreAppSubState = PRE_APP_KEY_READY;
		return;
	}

	if((gKeyRxContext.complete != 0u) && (gKeyRxContext.length == 0u))
	{
		EnterFailureState();
	}
}

static void HandlePreAppKeyReady()
{
	uint32_t authSize = (uint32_t)(_eauth_flash - _sauth_flash);

	ledSetLED(LED3, GPIO_PIN_SET);

	if(CopyAuthSectionToRam() != ERROR_OK)
	{
		EnterFailureState();
		return;
	}

	if(ProcessAuthDecryption(_sauth_ram, authSize, gKeyRxContext.buffer, gKeyRxContext.length) != ERROR_OK)
	{
		EnterFailureState();
		return;
	}

	gAuthState = STATE_START_APP;
}

static int32_t CopyAuthSectionToRam()
{
	uint32_t index = 0u;
	uint32_t authSize = (uint32_t)(_eauth_flash - _sauth_flash);

	// Return if .auth size is 0
	if(authSize == 0u) return ERROR_GENERAL;

	// Copying .auth section from FLASH to RAM
	for(index = 0; index < authSize; index++) _sauth_ram[index] = _sauth_flash[index];

	return ERROR_OK;
}

/**
 * @brief Decrypts the .auth section using XOR and the received key.
 */
static int32_t DecryptAuthSection(uint8_t* pData, uint32_t dataLength, const uint8_t* pKey, uint32_t keyLength)
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
static int32_t ProcessAuthDecryption(uint8_t* pData, uint32_t dataLength, const uint8_t* pKey, uint32_t keyLength)
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
	uint32_t index = 0u;

	if(pContext == NULL) return;

	for(index = 0u; index < UART_KEY_MAX_LENGTH; index++) pContext->buffer[index] = 0u;

	pContext->length		= 0u;
	pContext->complete		= 0u;
	pContext->overflow		= 0u;
	pContext->startTick		= HAL_GetTick();
	pContext->initialized	= PRE_APP_INITIALIZE_FLAG;
}

/**
 * @brief Function for Processing Key
 */
static void KeyRxProcess(KeyRxContext_t* pContext)
{
	int8_t hasChar = 0;
	uint8_t ch = 0;
	uint32_t receiveStatus = UART_ERR_OK;

	if((pContext == NULL) || (pContext->initialized == 0) || (pContext->complete != 0)) return;

	while(1)
	{
		hasChar = 0;
		uartHasData(&hasChar);

		if(hasChar == 0) break;

		receiveStatus = uartReceiveData(&ch, UART_RX_BYTE_COUNT);

		if(receiveStatus != UART_ERR_OK) break;

		if(ch == UART_KEY_TERMINATOR)
		{
			pContext->complete = KEY_COMPLETE_FLAG;
			break;
		}

		if(pContext->length < UART_KEY_MAX_LENGTH)
		{
			pContext->buffer[pContext->length] = ch;
			pContext->length++;
		}
		else
		{
			pContext->overflow = KEY_OVERFLOW_FLAG;
			break;
		}
	}
}

/**
 * @brief Function for getting Elapsed Time
 */
static uint32_t KeyRxGetElapsedTimeMs(const KeyRxContext_t* pContext)
{
	if((pContext == NULL) || (pContext->initialized == 0u)) return 0u;

	return HAL_GetTick() - pContext->startTick;
}

/**
 * @brief Function for updating Timeout Indicators
 */
static void UpdateKeyTimeoutIndication(uint32_t elapsedTimeMs)
{
	// Failure State after 45s
	if(elapsedTimeMs >= KEY_FAILURE_TIMEOUT_MS) EnterFailureState();

	// LED1 blinking after 30s
	else if(elapsedTimeMs >= KEY_STAGE2_TIMEOUT_MS)
	{
		if(((HAL_GetTick() / LED_FLASH_PERIOD_MS) % LED_FLASH_DIVIDER) == 0u) ledSetLED(LED1, GPIO_PIN_SET);

		else ledSetLED(LED1, GPIO_PIN_RESET);
	}

	// Turning on LED1 after 10s
	else if(elapsedTimeMs >= KEY_STAGE1_TIMEOUT_MS) ledSetLED(LED1, GPIO_PIN_SET);
}
