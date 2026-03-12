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
#include <stddef.h>
#include <stdint.h>

#include "AppTasks.h"
#include "Application.h"
#include "ButtonModule.h"

#include "DualChannelGasSensor.h"
#include "StackMonitor.h"


/***** PRIVATE CONSTANTS *****************************************************/
#define HEALTH_MONITOR_OK              0
#define HEALTH_MONITOR_ERR_STACK      -1


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static void checkButtonEvents(void);
static int32_t HealthCheck(void);

/***** PRIVATE VARIABLES *****************************************************/
static Button_Status_t gLastSw1State	= BUTTON_RELEASED;
static Button_Status_t gLastB1State		= BUTTON_RELEASED;


/***** PUBLIC FUNCTIONS ******************************************************/

/**
 * @brief 10 ms application task
 *
 */
void taskApp10ms()
{
	buttonCyclic10ms();
	toggleDashSymbol();
	checkButtonEvents();
	AppGasSensorHandler();
	waterSensorHandler();
}

/**
 * @brief 50 ms application task
 *
 */
void taskApp50ms()
{
	applicationRun();
	emergencyBlicking();
}

/**
 * @brief 250 ms application task
 *
 */
void taskApp250ms()
{
	int32_t healthCheck = HealthCheck();

	if (healthCheck != HEALTH_MONITOR_OK){

		applicationSendEvent(EVT_ID_STACK_CORRUPTION);

	}
}

/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Checks button states and generates corresponding application events
 *
 */
static void checkButtonEvents(void)
{
	Button_Status_t sw1State = buttonGetButtonStatus(BTN_SW1);
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
	gLastB1State = b1State;
}

/**
 * @brief Performs stack health monitoring checks
 *
 * @return Returns HEALTH_MONITOR_OK if no stack corruption was detected
 */
static int32_t HealthCheck(void)
{
    /* Kritische Prüfung */
    if (isCorrupted())
    {
        return HEALTH_MONITOR_ERR_STACK;
    }

    return HEALTH_MONITOR_OK;
}
