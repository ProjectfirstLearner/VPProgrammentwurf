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
#define UART_A_RX_FAILURE 15000

#define TIMER_START_VALUE 0
#define EMPTY_UART_VALUE 0

#define INITIALIZE_ZERO 0
#define INITIALIZE_ONE 1

#define UART_RX_ONE_BYTE 1
#define UART_TRIGGER_CHAR 'A'


/***** PRIVATE TYPES *********************************************************/

typedef enum
{
	STATE_BOOTUP = 0, //giving identification 0, 1-3 will be added automatically to the other states
	STATE_FAILURE,
	STATE_PRE_APP,
	STATE_START_APP
} AuthState_t;

/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals();
static void HandleBootupState();
static void HandlePreAppState();
static void HandleAppStartState();
static void HandleFailureState();

/***** PRIVATE VARIABLES *****************************************************/

static AuthState_t gAuthState = STATE_BOOTUP; //global within scope

static uint32_t gPreAppTickStart = TIMER_START_VALUE;
static uint8_t gPreAppInitialized = INITIALIZE_ZERO;

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
static void HandleBootupState() {

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
static void HandlePreAppState(){

	// Set the Timer for Timeout to 0
	uint32_t timeElapsed = TIMER_START_VALUE;

	// Initialize Tick for Timer only once
	if (gPreAppInitialized == INITIALIZE_ZERO){

		gPreAppTickStart = HAL_GetTick();
		gPreAppInitialized = INITIALIZE_ONE;
	}

	// Switching to Failure-State if the 15s timeout is reached
	timeElapsed = HAL_GetTick() - gPreAppTickStart;
	if(timeElapsed >= UART_A_RX_FAILURE)
	{
	    gAuthState = STATE_FAILURE;
	    return;
	}

	// Reading UART Input
	int8_t hasChar = EMPTY_UART_VALUE;
	uartHasData(&hasChar);

    if (hasChar != EMPTY_UART_VALUE)
    {
        uint8_t ch = EMPTY_UART_VALUE;
        uint32_t receiveStatus = uartReceiveData(&ch, UART_RX_ONE_BYTE);

        if ((receiveStatus == UART_ERR_OK) && (ch == UART_TRIGGER_CHAR))
        {
            gAuthState = STATE_START_APP;
            return;
        }
    }
}

/**
 * @brief Handles Application Start of authenticator
 */
static void HandleAppStartState(){
	//ledSetLED(LED0, GPIO_PIN_SET);
	//ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED2, GPIO_PIN_SET);
}

/**
 * @brief Handles Failure of authenticator
 */
static void HandleFailureState() {

	ledSetLED(LED0, GPIO_PIN_RESET);
	ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED2, GPIO_PIN_RESET);
	ledSetLED(LED4, GPIO_PIN_SET);
}
