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


/***** PRIVATE TYPES *********************************************************/

typedef enum{
	STATE_BOOTUP = 0, //giving identification 0, 1-3 will be added automatically to the other states
	STATE_FAILURE,
	STATE_PRE_APP,
	STATE_START_APP
}AuthState_t;

/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals();


/***** PRIVATE VARIABLES *****************************************************/

//static Scheduler gScheduler;            // Global Scheduler instance

static AuthState_t gAuthState = STATE_BOOTUP; //global within scope


/***** PUBLIC FUNCTIONS ******************************************************/


/**
 * @brief Main function of System
 */
int main(void)
{
	/*
    // Initialize the HAL
    HAL_Init();

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
    			break;
    		case STATE_PRE_APP:
    			//allpreStarting functions
    			break;
    		case STATE_START_APP:
    			//all functions to start app
    		case STATE_FAILURE:
    			//functions to handle Failure
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
    displayInitialize();

    // Initialize GPIOs for Buttons
    buttonInitialize();

    // Initialize Timer, DMA and ADC for sensor measurements
    timerInitialize();
    adcInitialize();

    return ERROR_OK;
}
