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


/*temporary*/

#include <stdint.h>
#include <stddef.h>


/*********GlobalObjects***************++/ */


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static void checkButtonEvents();

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
}


void taskApp50ms()
{
	applicationRun();
	outputLogf(" current state: %d ", applicationGetCurrentState());
	emergencyBlicking();
}

void taskApp250ms()
{

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
