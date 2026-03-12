/******************************************************************************
 * @file AppTasks.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation File for the application tasks
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "Scheduler.h"
#include "AppTasks.h"
#include "Application.h"
#include "GasSensor.h"
#include "ButtonModule.h"

#include "ADCModule.h"
#include "Util/Log/printf.h"
#include "Util/Log/LogOutput.h"

#include "DualChannelGasSensor.h"
#include "StackMonitor.h"

#include <stdint.h>
#include <stddef.h>


#define HEALTH_MONITOR_OK              0
#define HEALTH_MONITOR_ERR_STACK      -1

/*********GlobalObjects***************++/ */


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static void checkButtonEvents();
static int32_t HealthCheck(void);

/***** PRIVATE VARIABLES *****************************************************/

static Button_Status_t gLastSw1State	= BUTTON_RELEASED;
//static Button_Status_t gLastSw2State	= BUTTON_RELEASED;
static Button_Status_t gLastB1State		= BUTTON_RELEASED;

/***PRIVATE FUNCTIONS***/

#define APP_ADC_MAX_VALUE       4095U
#define APP_ADC_REFERENCE_UV    3300000U


/***** PUBLIC FUNCTIONS ******************************************************/

void taskApp10ms()
{
	buttonCyclic10ms();
	checkButtonEvents();
	AppGasSensorHandler();
	waterSensorHandler();
}


void taskApp50ms()
{
	applicationRun();
	outputLogf(" current state: %d ", applicationGetCurrentState());
	emergencyBlicking();
}

void taskApp250ms()
{
	int32_t healthCheck = HealthCheck();

	if (healthCheck != HEALTH_MONITOR_OK){

		applicationSendEvent(EVT_ID_STACK_CORRUPTION);

	}
}

/***** PRIVATE FUNCTIONS *****************************************************/
static void checkButtonEvents()
{
	Button_Status_t sw1State = buttonGetButtonStatus(BTN_SW1);
	//Button_Status_t sw2State = buttonGetButtonStatus(BTN_SW2);
	Button_Status_t b1State  = buttonGetButtonStatus(BTN_B1);

	// Get the current active State
	int32_t currentState = applicationGetCurrentState();

	/* SW1: Pre-Operational <-> Operational */
	if((gLastSw1State == BUTTON_RELEASED) && (sw1State == BUTTON_PRESSED))
	{

		if(currentState == STATE_ID_PRE_OPERATIONAL)
			applicationSendEvent(EVT_ID_SWITCH_TO_OPERATIONAL);

		else if(currentState == STATE_ID_OPERATIONAL)
			applicationSendEvent(EVT_ID_SWITCH_TO_PRE_OPERATIONAL);

	}

	if((gLastB1State == BUTTON_RELEASED) && (b1State == BUTTON_PRESSED))
	{
		if(currentState == STATE_ID_EMERGENCY)
		{
			applicationSendEvent(EVT_ID_ALARM_RESET);
		}
	}

	gLastSw1State = sw1State;
}



static int32_t HealthCheck(void)
{
    int32_t freeBytes = getFreeBytes();
    int32_t usedBytes = getUsedBytes();
    int32_t usage     = getUsage();



    //outputLogf("Stack free: %ld bytes\r\n", getFreeBytes());
    //outputLogf("Stack used: %ld bytes\r\n", getUsedBytes());
    //outputLogf("Stack usage: %ld %%\r\n", getUsage());

    /* Kritische Prüfung */
    if (isCorrupted())
    {
    	outputLogf("ERROR: Stack corruption detected!\r\n");
        return HEALTH_MONITOR_ERR_STACK;
    }

    return HEALTH_MONITOR_OK;
}
