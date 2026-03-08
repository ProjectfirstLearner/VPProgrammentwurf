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
#include "ButtonModule.h"
#include "LEDModule.h"
#include "DisplayModule.h"
#include "ADCModule.h"
#include "TimerModule.h"
#include "Scheduler.h"

#include "GlobalObjects.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/
#define UART_A_RX_FAILURE 15000U

#define TIMER_START_VALUE 0
#define EMPTY_UART 0
#define INITIALIZE_FALSE 0
#define INITIALIZE_TRUE 1


/***** PRIVATE TYPES *********************************************************/

typedef enum{
	STATE_BOOTUP = 0, //giving identification 0, 1-3 will be added automatically to the other states
	STATE_FAILURE,
	STATE_PRE_APP,
	STATE_START_APP
}AuthState_t;

/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals();
static void HandleBootupState();
static void HandlePreAppState();
static void HandleAppStartState();
static void HandleFailureState();

/***** PRIVATE VARIABLES *****************************************************/

//static Scheduler gScheduler;            // Global Scheduler instance

static AuthState_t gAuthState = STATE_BOOTUP; //global within scope

static uint32_t gPreAppTickStart = 0U;
static uint8_t gPreAppInitialized = 0U;


/***** PUBLIC FUNCTIONS ******************************************************/


/**
 * @brief Main function of System
 */
int main(void)
{
	/*
    // Initialize the HAL
    HAL_Init();
eAppTickStart = 0U;
    SystemClock_Config();

    // Initialize Peripherals
    initializePeripherals();

    // Initialize Scheduler
    schedInitialize(&gScheduler);
	*/

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


    	/*
        // Toggle all LEDs to the their functionality (Toggle frequency depends on HAL_Delay at end of loop)
        ledToggleLED(LED0);
        HAL_Delay(100);
        ledToggleLED(LED1);
        HAL_Delay(100);
        ledToggleLED(LED2);
        HAL_Delay(100);
        ledToggleLED(LED3);
        HAL_Delay(100);
        ledToggleLED(LED4);
        HAL_Delay(100);

        uint8_t buf[10];
        uartReceiveData(buf, 2);
        if (buf[0] == 'X' && buf[1] == '\r')
        {
            displayShowDigit(RIGHT_DISPLAY, DIGIT_DASH);
        }
        else
        {
            displayShowDigit(RIGHT_DISPLAY, DIGIT_OFF);
        }
        */
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
    //displayInitialize();

    // Initialize GPIOs for Buttons
    //buttonInitialize();

    // Initialize Timer, DMA and ADC for sensor measurements
    //timerInitialize();
    //adcInitialize();

    return ERROR_OK;
}
/**
 * @brief Handles bootup of authenticator
 */
static void HandleBootupState() {

	HAL_StatusTypeDef halStatus;

	halStatus = HAL_Init();
	// If HAL or Peripheral Initialization was not Successful: State -> FAILURE
	if(halStatus != HAL_OK) {
		gAuthState = STATE_FAILURE;
		return;
	}

	// Set correct Clock Timing
	SystemClock_Config();

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

static void HandlePreAppState(){


	uint32_t timeElapsed = TIMER_START_VALUE;

	if (gPreAppInitialized == INITIALIZE_FALSE){

		gPreAppTickStart = HAL_GetTick();
		gPreAppInitialized = INITIALIZE_TRUE;

		//D1 should stay on rest off until time elapsed
		ledSetLED(LED0, GPIO_PIN_SET);
		ledSetLED(LED1, GPIO_PIN_RESET);
		ledSetLED(LED2, GPIO_PIN_RESET);

	}

	//switching to failure if the 15s time is elapsed and no A input is given
	timeElapsed = HAL_GetTick() - gPreAppTickStart;
	if(timeElapsed >= UART_A_RX_FAILURE)
	{
	    gAuthState = STATE_FAILURE;
	    return;
	}

	int8_t hasChar = EMPTY_UART;
	    uartHasData(&hasChar);

	    if (hasChar) {
	        uint8_t ch = EMPTY_UART;
	        uint32_t receiveOK = uartReceiveData(&ch, 1);

	        // Checking for Input 'A' in Uart-Buffer
	        if (receiveOK == UART_ERR_OK && ch == 'A') {
	                gAuthState = STATE_START_APP;
	                return;
	        }

	    }
}

static void HandleAppStartState(){
	ledSetLED(LED0, GPIO_PIN_RESET);
	ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED2, GPIO_PIN_SET);
}

static void HandleFailureState() {

	ledSetLED(LED0, GPIO_PIN_RESET);
	ledSetLED(LED1, GPIO_PIN_RESET);
	ledSetLED(LED4, GPIO_PIN_SET);
}
