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
 * @brief Main file for the VP Template project
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
#include "AppTasks.h"
#include "App/Application.h"
#include "GasSensor.h"
#include "Util/Filter/Filter.h"

/***** PRIVATE CONSTANTS *****************************************************/
const char signature[] __attribute__ ((section (".signature"))) = "UMMS";

/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals(void);


/***** PRIVATE VARIABLES *****************************************************/

/* Global scheduler instance */
static Scheduler gScheduler;


/***** PUBLIC FUNCTIONS ******************************************************/

/**
 * @brief Main function of the application
 *
 * @return Never returns
 */
int main(void)
{
	__enable_irq();
	HAL_DeInit();

	__HAL_RCC_AHB1_FORCE_RESET();
	__HAL_RCC_AHB1_RELEASE_RESET();

	/* Initialize HAL */
    HAL_Init();

    /* Initialize system clock */
    SystemClock_Config();

    /* Initialize peripherals */
    initializePeripherals();

    /* Initialize scheduler */
    schedInitialize(&gScheduler);

    /* Initialize application state machine */
    applicationInitialize();

    /* Assign scheduler time base */
    gScheduler.pGetHALTick = HAL_GetTick;

    /* Assign scheduler task intervals */
    gScheduler.pTask_10ms = taskApp10ms;
    gScheduler.pTask_50ms = taskApp50ms;
    gScheduler.pTask_250ms = taskApp250ms;

    while(1)
    {
    	schedCycle(&gScheduler);
    }
}

/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Initializes the used peripherals like GPIO, ADC, DMA and timer
 *
 * @return Returns ERROR_OK if no error occurred
 */
static int32_t initializePeripherals()
{
	/* Initialize UART used for debug outputs */
    uartInitialize(115200);

    /* Initialize GPIOs for LEDs and 7-segment display */
	ledInitialize();
    displayInitialize();

    /* Initialize GPIOs for buttons */
    buttonInitialize();

    /* Initialize timer, DMA and ADC for sensor measurements */
    timerInitialize();
    adcInitialize();

    return ERROR_OK;
}
