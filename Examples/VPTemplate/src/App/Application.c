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

/******************************GlobalObjects***********************************/

static StateTable_t gStateTable;

/***** PRIVATE PROTOTYPES ****************************************************/

//defining functions that should be executing when in according state
static int32_t onEntryInitialization(State_t* pState, int32_t eventID);
static int32_t onStateInitialization(State_t* pState, int32_t eventID);
static int32_t onStatePreOperational(State_t* pState, int32_t eventID);
static int32_t onStateOperational(State_t* pState, int32_t eventID);
static int32_t onStateEmergency(State_t* pState, int32_t eventID);
static int32_t onStateTestMode(State_t* pState, int32_t eventID);
static int32_t onStateFailure(State_t* pState, int32_t eventID);

/***** PRIVATE VARIABLES *****************************************************/

//asigning the functions to states
static State_t gStateList[] =
{
    {STATE_ID_INITIALIZATION,  onEntryInitialization, 	onStateInitialization, NULL, false},
    {STATE_ID_PRE_OPERATIONAL, NULL, 					onStatePreOperational, NULL, false},
    {STATE_ID_OPERATIONAL,     NULL, 					onStateOperational,    NULL, false},
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
    {STATE_ID_TEST_MODE,       STATE_ID_FAILURE,         EVT_ID_STACK_CORRUPTION,          NULL, NULL, NULL}
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

/***** PRIVATE FUNCTIONS *****************************************************/

static int32_t onEntryInitialization(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    ledSetLED(LED0, LED_ON);
    ledSetLED(LED1, LED_OFF);
    ledSetLED(LED2, LED_OFF);
    ledSetLED(LED3, LED_OFF);
    ledSetLED(LED4, LED_OFF);
    HAL_Delay(5000);
    applicationSendEvent(EVT_ID_INIT_READY);
    return ERROR_OK;
}

static int32_t onStateInitialization(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

static int32_t onStatePreOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    ledSetLED(LED0, LED_OFF);
    ledSetLED(LED1, LED_ON);
    ledSetLED(LED2, LED_OFF);
    ledSetLED(LED3, LED_OFF);
    ledSetLED(LED4, LED_OFF);
    applicationSendEvent(EVT_ID_SWITCH_TO_OPERATIONAL);
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

    ledSetLED(LED0, LED_OFF);
    ledSetLED(LED1, LED_OFF);
    ledSetLED(LED2, LED_OFF);
    ledSetLED(LED3, LED_OFF);
    ledSetLED(LED4, LED_ON);

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
	}

	return ERROR_OK;
}
