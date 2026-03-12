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
#include "string.h"

#include "stm32g4xx_hal.h"

#include "System.h"
#include "HardwareConfig.h"

#include "Util/Global.h"

#include "UARTModule.h"
#include "LEDModule.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/

/* Timeout constants */
#define UART_A_RX_FAILURE_MS			15000u
#define KEY_STAGE1_TIMEOUT_MS			10000u
#define KEY_STAGE2_TIMEOUT_MS			30000u
#define KEY_FAILURE_TIMEOUT_MS			45000u

/* UART constants */
#define UART_RX_BYTE_COUNT				1u
#define UART_TRIGGER_CHAR				'A'
#define UART_KEY_TERMINATOR				'\r'
#define UART_KEY_MAX_LENGTH				8u

/* Initialization flags */
#define PRE_APP_INITIALIZE_FLAG			1u
#define KEY_COMPLETE_FLAG				1u
#define KEY_OVERFLOW_FLAG				1u

/* Key limits */
#define KEY_LENGTH_MIN_VALUE			0u
#define KEY_LENGTH_MAX_VALUE			8u

/* LED control */
#define LED_FLASH_PERIOD_MS				250u
#define LED_FLASH_DIVIDER				2u

/* Application and signature addresses */
#define APPLICATION_FLASH_ADDRESS		0x08010000u
#define APPLICATION_START_ADDRESS		0x08010204u
#define SIGNATURE_BYTE_0				'U'
#define SIGNATURE_BYTE_1				'M'
#define SIGNATURE_BYTE_2				'M'
#define SIGNATURE_BYTE_3				'S'

/***** PRIVATE TYPES *********************************************************/

/**
 * @brief State machine states of the authenticator
 *
 */
typedef enum
{
	STATE_BOOTUP = 0,
	STATE_FAILURE,
	STATE_PRE_APP,
	STATE_START_APP
} AuthState_t;

/**
 * @brief Sub-states for the pre-application phase
 *
 */
typedef enum
{
	PRE_APP_WAIT_FOR_TRIGGER = 0,
	PRE_APP_RECEIVE_KEY,
	PRE_APP_KEY_READY
} PreAppSubState_t;

/**
 * @brief Context structure for UART key reception
 *
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

/**
 * @brief Function pointer type for functions in decrypted auth section
 *
 */
typedef void (*AuthFunction_t)();

/**
 * @brief Function pointer type for application start function
 *
 */
typedef void (*ApplicationFunction_t)();

/***** PRIVATE PROTOTYPES ****************************************************/

static int32_t initializePeripherals(void);

static void EnterFailureState(void);
static void EnterPreAppState(void);

/* State machine functions */
static void HandleBootupState(void);
static void HandlePreAppState(void);
static void HandleAppStartState(void);
static void HandleFailureState(void);

/* Pre-application helper functions */
static void HandlePreAppWaitForTrigger(void);
static void HandlePreAppReceiveKey(void);
static void HandlePreAppKeyReady(void);

/* Auth section copy / decrypt */
static int32_t CopyDecryptAuthSection(const uint8_t* pKey, uint32_t keyLength);

/* Key receive functions */
static void KeyRxInit(KeyRxContext_t* pContext);
static void KeyRxProcess(KeyRxContext_t* pContext);
static uint32_t KeyRxGetElapsedTimeMs(const KeyRxContext_t* pContext);
static void UpdateKeyTimeoutIndication(uint32_t elapsedTimeMs);

/* Verify functions */
static int32_t ExecuteVerifyFromRam(void);
void verify() __attribute__((section(".auth"), noinline));

/***** PRIVATE VARIABLES *****************************************************/

static AuthState_t gAuthState = STATE_BOOTUP;

static uint32_t gPreAppTickStart = 0u;
static uint8_t gPreAppInitialized = 0u;

static PreAppSubState_t gPreAppSubState = PRE_APP_WAIT_FOR_TRIGGER;
static KeyRxContext_t gKeyRxContext = {0};

/* Auth section variables provided by linker */
extern uint8_t _sauth;
extern uint8_t _eauth;
extern uint8_t _sloadauth;

/***** PUBLIC FUNCTIONS ******************************************************/

/**
 * @brief Main function of the authenticator
 *
 * @return Never returns
 */
int main(void)
{
    while (1)
    {
    	switch (gAuthState){

    		case STATE_BOOTUP:
    			HandleBootupState();
    			break;

    		case STATE_PRE_APP:
    			HandlePreAppState();
    			break;

    		case STATE_START_APP:
    			HandleAppStartState();
    			break;

    		case STATE_FAILURE:
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
static int32_t initializePeripherals(void)
{
	/* Initialize UART used for debug outputs */
    uartInitialize(115200);

    /* Initialize GPIOs for LED output */
	ledInitialize();

    return ERROR_OK;
}

/**
 * @brief Enters authenticator failure state
 *
 */
static void EnterFailureState()
{
	gAuthState = STATE_FAILURE;
}

/**
 * @brief Enters pre-application state and initializes related state variables
 *
 */
static void EnterPreAppState()
{
	gPreAppTickStart = HAL_GetTick();
	gPreAppInitialized = PRE_APP_INITIALIZE_FLAG;
	gPreAppSubState = PRE_APP_WAIT_FOR_TRIGGER;
	gAuthState = STATE_PRE_APP;
}

/**
 * @brief Handles boot-up state of the authenticator
 *
 */
static void HandleBootupState()
{
	if(HAL_Init() != HAL_OK)
	{
		EnterFailureState();
		return;
	}

	SystemClock_Config();

	if(initializePeripherals() != ERROR_OK)
	{
		EnterFailureState();
		return;
	}

	ledSetLED(LED0, GPIO_PIN_SET);

	EnterPreAppState();
}

/**
 * @brief Handles pre-application state of the authenticator
 *
 */
static void HandlePreAppState()
{
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
 * @brief Handles application start state of the authenticator
 *
 */
static void HandleAppStartState()
{
	if(ExecuteVerifyFromRam() != ERROR_OK)
	{
		EnterFailureState();
		return;
	}

	EnterFailureState();
}

/**
 * @brief Handles failure state of the authenticator
 *
 */
static void HandleFailureState()
{
	ledSetLED(LED0, GPIO_PIN_RESET);
	ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED4, GPIO_PIN_SET);
}

/**
 * @brief Handles trigger wait sub-state in pre-application phase
 *
 */
static void HandlePreAppWaitForTrigger()
{
	uint32_t timeElapsed = HAL_GetTick() - gPreAppTickStart;
	int8_t hasChar = 0;
	uint8_t ch = 0;
	uint32_t receiveStatus = UART_ERR_OK;

	if(timeElapsed >= UART_A_RX_FAILURE_MS)
	{
		EnterFailureState();
		return;
	}

	uartHasData(&hasChar);

	if (hasChar == 0) return;

	receiveStatus = uartReceiveData(&ch, UART_RX_BYTE_COUNT);
	if ((receiveStatus == UART_ERR_OK) && (ch == (uint8_t)UART_TRIGGER_CHAR))
	{
		KeyRxInit(&gKeyRxContext);
		gPreAppSubState = PRE_APP_RECEIVE_KEY;
	}
}

/**
 * @brief Handles key receive sub-state in pre-application phase
 *
 */
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

	if((gKeyRxContext.complete != 0u) && (gKeyRxContext.length > KEY_LENGTH_MIN_VALUE) && (gKeyRxContext.length <= KEY_LENGTH_MAX_VALUE))
	{
		gPreAppSubState = PRE_APP_KEY_READY;
		return;
	}

	if((gKeyRxContext.complete != 0u) && (gKeyRxContext.length == 0u))
	{
		EnterFailureState();
	}
}

/**
 * @brief Handles key ready sub-state in pre-application phase
 *
 */
static void HandlePreAppKeyReady()
{
	if(CopyDecryptAuthSection(gKeyRxContext.buffer, gKeyRxContext.length) != ERROR_OK)
	{
		EnterFailureState();
		return;
	}

	gAuthState = STATE_START_APP;
}

/**
 * @brief Copies encrypted .auth section to RAM and decrypts it using XOR
 *
 * @param pKey Pointer to received decryption key
 * @param keyLength Length of the received key
 *
 * @return Returns ERROR_OK if no error occurred
 */
static int32_t CopyDecryptAuthSection(const uint8_t* pKey, uint32_t keyLength)
{
	uint8_t *dst = &_sauth;
	uint8_t *src = &_sloadauth;

	size_t section_length = (size_t)(&_eauth - &_sauth);

	memcpy(dst, src, section_length);

	for(size_t i = 0; i < section_length; i++)
	{
		dst[i] ^= pKey[i % keyLength];
	}

	__DSB();
	__ISB();

	return ERROR_OK;
}

/**
 * @brief Initializes key reception context
 *
 * @param pContext Pointer to key reception context
 *
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
 * @brief Processes incoming UART characters for key reception
 *
 * @param pContext Pointer to key reception context
 *
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
 * @brief Returns elapsed time for current key reception
 *
 * @param pContext Pointer to key reception context
 *
 * @return Returns elapsed time in milliseconds
 */
static uint32_t KeyRxGetElapsedTimeMs(const KeyRxContext_t* pContext)
{
	if((pContext == NULL) || (pContext->initialized == 0u)) return 0u;

	return HAL_GetTick() - pContext->startTick;
}

/**
 * @brief Updates LED indication for key receive timeout stages
 *
 * @param elapsedTimeMs Elapsed key reception time in milliseconds
 *
 */
static void UpdateKeyTimeoutIndication(uint32_t elapsedTimeMs)
{
	if(elapsedTimeMs >= KEY_FAILURE_TIMEOUT_MS) EnterFailureState();

	else if(elapsedTimeMs >= KEY_STAGE2_TIMEOUT_MS)
	{
		if(((HAL_GetTick() / LED_FLASH_PERIOD_MS) % LED_FLASH_DIVIDER) == 0u) ledSetLED(LED1, GPIO_PIN_SET);

		else ledSetLED(LED1, GPIO_PIN_RESET);
	}

	else if(elapsedTimeMs >= KEY_STAGE1_TIMEOUT_MS) ledSetLED(LED1, GPIO_PIN_SET);
}

/**
 * @brief Executes verify() function from decrypted RAM-based .auth section
 *
 * @return Returns ERROR_OK if no error occurred
 */
static int32_t ExecuteVerifyFromRam()
{
	uint32_t verifyAddress = 0u;
	AuthFunction_t verifyFunction = (AuthFunction_t)0;

	verifyAddress = (uint32_t)&_sauth;
	verifyAddress |= 0x1u;

	verifyFunction = (AuthFunction_t)verifyAddress;
	verifyFunction();

	return ERROR_OK;
}

/**
 * @brief Verifies application signature and starts the application
 *
 */
__attribute__((section(".auth"), noinline))
void verify(void)
{
	volatile const uint8_t* pSignature = (const uint8_t*)APPLICATION_FLASH_ADDRESS;

	if (pSignature[0] != SIGNATURE_BYTE_0) return;
	if (pSignature[1] != SIGNATURE_BYTE_1) return;
	if (pSignature[2] != SIGNATURE_BYTE_2) return;
	if (pSignature[3] != SIGNATURE_BYTE_3) return;

	__disable_irq();

	uint32_t *applicationStartAddress = (uint32_t*)(APPLICATION_START_ADDRESS);
	ApplicationFunction_t applicationStart = (ApplicationFunction_t)*(applicationStartAddress);

	applicationStart();
}
