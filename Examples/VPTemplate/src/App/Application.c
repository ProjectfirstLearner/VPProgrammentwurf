/******************************************************************************
 * @file Application.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation file for main application (state machine)
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include <string.h>

#include "Application.h"//app implementation
#include "Util/Global.h"//error codes
#include "Util/Log/printf.h"

#include "UARTModule.h"
#include "ButtonModule.h"
#include "LEDModule.h"

#include "Util/StateTable/StateTable.h" //state table
#include "DualChannelGasSensor.h"
#include "WaterSensor.h"

#include "DisplayModule.h"

/***** PRIVATE CONSTANTS *****************************************************/

#define CURRENT_STATE_ERROR			-1

#define WARNING_THRESHOLD			3000
#define EMERGENCY_THRESHOLD			5000

#define WARNING_TIME_ELAPSED		5000
#define EMERGENCY_TIME_ELAPSED		3000

#define WATER_VOLTAGE_PLACEHOLDER	7000

#define HUNDRETS					100
#define TENS						10

#define MAX_WATER_VALUE				1000
#define MAX_ACCEPTED_VALUE			999


static int32_t emergencyTimer = 0;
static int32_t warningTimer = 0;

/******************************GlobalObjects***********************************/

static StateTable_t gStateTable;

/***** PRIVATE PROTOTYPES ****************************************************/

//defining functions that should be executing when in according state
static int32_t onEntryInitialization(State_t* pState, int32_t eventID);
static int32_t onStateInitialization(State_t* pState, int32_t eventID);
static int32_t onEntryPreOperational(State_t* pState, int32_t eventID);
static int32_t onStatePreOperational(State_t* pState, int32_t eventID);
static int32_t onEntryOperational(State_t* pState, int32_t eventID);
static int32_t onStateOperational(State_t* pState, int32_t eventID);
static int32_t onStateEmergency(State_t* pState, int32_t eventID);
static int32_t onStateTestMode(State_t* pState, int32_t eventID);
static int32_t onStateFailure(State_t* pState, int32_t eventID);

/***** PRIVATE VARIABLES *****************************************************/

//asigning the functions to states
static State_t gStateList[] =
{
    {STATE_ID_INITIALIZATION,  onEntryInitialization, 	onStateInitialization, NULL, false},
    {STATE_ID_PRE_OPERATIONAL, onEntryPreOperational,	onStatePreOperational, NULL, false},
    {STATE_ID_OPERATIONAL,     onEntryOperational,		onStateOperational,    NULL, false},
    {STATE_ID_EMERGENCY,       NULL, 					onStateEmergency,      NULL, false},
    {STATE_ID_TEST_MODE,       NULL, 					onStateTestMode,       NULL, false},
    {STATE_ID_FAILURE,         NULL, 					onStateFailure,        NULL, false}
};

//State table, defining from which state can be switched to which
static StateTableEntry_t gStateTableEntries[] =
{
    {STATE_ID_INITIALIZATION,  STATE_ID_PRE_OPERATIONAL, EVT_ID_INIT_READY,                NULL, NULL, NULL},
    {STATE_ID_INITIALIZATION,  STATE_ID_FAILURE,         EVT_ID_SENSOR_FAILED,             NULL, NULL, NULL},
    {STATE_ID_INITIALIZATION,  STATE_ID_FAILURE,         EVT_ID_STACK_CORRUPTION,          NULL, NULL, NULL},

    {STATE_ID_PRE_OPERATIONAL, STATE_ID_OPERATIONAL,     EVT_ID_SWITCH_TO_OPERATIONAL,     NULL, NULL, NULL},
    {STATE_ID_PRE_OPERATIONAL, STATE_ID_TEST_MODE,       EVT_ID_TEST_MODE_TRIGGERED,       NULL, NULL, NULL},
    {STATE_ID_PRE_OPERATIONAL, STATE_ID_FAILURE,         EVT_ID_STACK_CORRUPTION,          NULL, NULL, NULL},

    {STATE_ID_OPERATIONAL,     STATE_ID_PRE_OPERATIONAL, EVT_ID_SWITCH_TO_PRE_OPERATIONAL, NULL, NULL, NULL},
    {STATE_ID_OPERATIONAL,     STATE_ID_EMERGENCY,       EVT_ID_EMERGENCY_TRIGGERED,       NULL, NULL, NULL},
    {STATE_ID_OPERATIONAL,     STATE_ID_TEST_MODE,       EVT_ID_TEST_MODE_TRIGGERED,       NULL, NULL, NULL},
    {STATE_ID_OPERATIONAL,     STATE_ID_FAILURE,         EVT_ID_SENSOR_FAILED,             NULL, NULL, NULL},
    {STATE_ID_OPERATIONAL,     STATE_ID_FAILURE,         EVT_ID_STACK_CORRUPTION,          NULL, NULL, NULL},

    {STATE_ID_EMERGENCY,       STATE_ID_OPERATIONAL,     EVT_ID_ALARM_RESET,               NULL, NULL, NULL},
    {STATE_ID_EMERGENCY,       STATE_ID_FAILURE,         EVT_ID_SENSOR_FAILED,             NULL, NULL, NULL},
    {STATE_ID_EMERGENCY,       STATE_ID_FAILURE,         EVT_ID_STACK_CORRUPTION,          NULL, NULL, NULL},

    {STATE_ID_TEST_MODE,       STATE_ID_FAILURE,         EVT_ID_SENSOR_FAILED,             NULL, NULL, NULL},
};



/***** PUBLIC FUNCTIONS ******************************************************/

//function on initializacion

int32_t applicationInitialize(void)
{
	DualChannelInit();

    gStateTable.pStateList = gStateList;														//adding states to global instance
    gStateTable.stateCount = sizeof(gStateList) / sizeof(gStateList[0]);						//

    return stateTableInitialize(&gStateTable,													/*calculating state count so that
     	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	the framework can work safely */
                                gStateTableEntries,
                                sizeof(gStateTableEntries) / sizeof(gStateTableEntries[0]),		//using size of arrays so that when adding the calc still works
																								//exmpl. when checking for valid state switch, iterating till statecount works
                                STATE_ID_INITIALIZATION);									//starting state
}

int32_t applicationRun(void)
{
    return stateTableRunCyclic(&gStateTable);	//function to be calling the state table (every 50ms)
}

int32_t applicationSendEvent(int32_t eventID)
{
    return stateTableSendEvent(&gStateTable, eventID);		//function to sending event
}

int32_t applicationGetCurrentState()
{
	if(gStateTable.pCurrentStateRef == NULL) return CURRENT_STATE_ERROR;

	return gStateTable.pCurrentStateRef->stateID;
}

/***** PRIVATE FUNCTIONS *****************************************************/

static int32_t onEntryInitialization(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    applicationSendEvent(EVT_ID_INIT_READY);
    return ERROR_OK;
}

static int32_t onStateInitialization(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

static int32_t onEntryPreOperational(State_t* pState, int32_t eventID)
{
	ledSetLED(LED0, LED_OFF);

	return ERROR_OK;
}

static int32_t onStatePreOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

static int32_t onEntryOperational(State_t* pState, int32_t eventID)
{
	ledSetLED(LED0, LED_ON);


	return ERROR_OK;
}

static int32_t onStateOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;
    return ERROR_OK;
}

static int32_t onStateEmergency(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

static int32_t onStateTestMode(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;
    return ERROR_OK;
}

static int32_t onStateFailure(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;
    return ERROR_OK;
}

int32_t AppGasSensorHandler(void)
{

	if(gStateTable.currentStateID == STATE_ID_OPERATIONAL)
	{
		gasSensorHandler();
		ppmThresholdChecking();

	}
	return ERROR_OK;
}

int32_t emergencyBlicking(){

	if (gStateTable.currentStateID == STATE_ID_EMERGENCY){
		ledToggleLED(LED1);
	}

	return ERROR_OK;
}


int32_t ppmThresholdChecking(){

	int32_t now = HAL_GetTick();


	uint32_t avrg = 0;
	getAvrg(&avrg);

	if (avrg > EMERGENCY_THRESHOLD){

		if (emergencyTimer ==0){

			emergencyTimer = now;

		}

		uint32_t elapsed = now -emergencyTimer;

		if (elapsed > EMERGENCY_TIME_ELAPSED ){

			applicationSendEvent(EVT_ID_EMERGENCY_TRIGGERED);

			return DUAL_SENSOR_OK;
		}

	}
	else {
		emergencyTimer = 0;
	}
	if (avrg > WARNING_THRESHOLD){

			if (warningTimer ==0){

				warningTimer = now;

			}

			if ((now-warningTimer) > WARNING_TIME_ELAPSED )
			{
				ledSetLED(LED1, LED_ON);
				return DUAL_SENSOR_OK;
			}
		}
		else {
			warningTimer = 0;
		}
	return DUAL_SENSOR_OK;

}

int32_t waterSensorHandler()
{
	if(gStateTable.currentStateID == STATE_ID_OPERATIONAL){

    WaterSensorSetSensorVoltage(WATER_VOLTAGE_PLACEHOLDER);

    uint32_t waterLevel = 0;

    int32_t waterValueCheck = WaterSensorGetSensorValue(&waterLevel);

    static uint8_t displayToggle = 0;

    if (waterValueCheck == SENSOR_OK)
    {
        /* Limit value to 999 */
        if (waterLevel >= MAX_WATER_VALUE)
        {
            waterLevel = MAX_ACCEPTED_VALUE;
        }

        int32_t hundreds = waterLevel / HUNDRETS;
        int32_t tens = (waterLevel / TENS) % TENS;

        if (displayToggle == 0)
        {
            displayShowDigit(LEFT_DISPLAY, hundreds);
            displayToggle = 1;
        }
        else
        {
            displayShowDigit(RIGHT_DISPLAY, tens);
            displayToggle = 0;
        }
    }
	}
    return SENSOR_OK;


}


