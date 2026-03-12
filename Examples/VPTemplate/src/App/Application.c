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
#include "Application.h"

#include "Util/Global.h"
#include "Util/StateTable/StateTable.h"

#include "WaterSensor.h"
#include "LEDModule.h"
#include "DisplayModule.h"
#include "DualChannelGasSensor.h"

#include "stm32g4xx_hal.h"

/***** PRIVATE CONSTANTS *****************************************************/
#define CURRENT_STATE_ERROR			-1

#define WARNING_THRESHOLD			3000
#define EMERGENCY_THRESHOLD			5000

#define WARNING_TIME_ELAPSED		5000
#define EMERGENCY_TIME_ELAPSED		3000

#define WATER_VOLTAGE_PLACEHOLDER	700000U

#define HUNDRETS					100
#define TENS						10

#define MAX_WATER_VALUE				1000
#define MAX_ACCEPTED_VALUE			999

#define DASH_DIGIT					16

/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/

/* State handler functions */
static int32_t onStateInitialization(State_t* pState, int32_t eventID);
static int32_t onEntryPreOperational(State_t* pState, int32_t eventID);
static int32_t onStatePreOperational(State_t* pState, int32_t eventID);
static int32_t onEntryOperational(State_t* pState, int32_t eventID);
static int32_t onStateOperational(State_t* pState, int32_t eventID);
static int32_t onStateEmergency(State_t* pState, int32_t eventID);
static int32_t onExitEmergency(State_t* pState, int32_t eventID);
static int32_t onEntryFailure(State_t* pState, int32_t eventID);
static int32_t onStateFailure(State_t* pState, int32_t eventID);

/***** PRIVATE VARIABLES *****************************************************/
static int32_t emergencyTimer = 0;
static int32_t warningTimer = 0;
static int32_t now = 0;
static uint32_t elapsed = 0u;

int8_t displayToggle = 0;

static StateTable_t gStateTable;

/**
 * @brief State list of the application state machine
 *
 */
static State_t gStateList[] =
{
    {STATE_ID_INITIALIZATION,  NULL,				 	onStateInitialization, NULL,			false},
    {STATE_ID_PRE_OPERATIONAL, onEntryPreOperational,	onStatePreOperational, NULL,			false},
    {STATE_ID_OPERATIONAL,     onEntryOperational,		onStateOperational,    NULL,			false},
    {STATE_ID_EMERGENCY,       NULL, 					onStateEmergency,      onExitEmergency,	false},
    {STATE_ID_TEST_MODE,       NULL, 					NULL,			       NULL,			false},
    {STATE_ID_FAILURE,         onEntryFailure,			onStateFailure,        NULL,			false}
};

/**
 * @brief State transition table of the application state machine
 *
 */
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

/**
 * @brief Initializes application state machine
 *
 * @return Returns result of state table initialization
 */
int32_t applicationInitialize(void)
{

    gStateTable.pStateList = gStateList;														//adding states to global instance
    gStateTable.stateCount = sizeof(gStateList) / sizeof(gStateList[0]);

    return stateTableInitialize(&gStateTable,													/* calculating state count so that
     	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	the framework can work safely */
                                gStateTableEntries,
                                sizeof(gStateTableEntries) / sizeof(gStateTableEntries[0]),		// using size of arrays so that when adding the calc still works
																								// exmpl. when checking for valid state switch, iterating till statecount works
                                STATE_ID_INITIALIZATION);										// starting state
}

/**
 * @brief Runs application state machine cyclically
 *
 * @return Returns result of cyclic state machine execution
 */
int32_t applicationRun(void)
{
    return stateTableRunCyclic(&gStateTable);	// function to call the state table (every 50ms)
}

/**
 * @brief Sends an event to the application state machine
 *
 * @param eventID Event ID to send
 *
 * @return Returns result of event handling
 */
int32_t applicationSendEvent(int32_t eventID)
{
    return stateTableSendEvent(&gStateTable, eventID);		//function to send event
}

/**
 * @brief Returns current active state of the application
 *
 * @return Returns state ID or CURRENT_STATE_ERROR
 */
int32_t applicationGetCurrentState()
{
	if(gStateTable.pCurrentStateRef == NULL) return CURRENT_STATE_ERROR;

	return gStateTable.pCurrentStateRef->stateID;
}


/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief State handler for initialization state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onStateInitialization(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    DualChannelInit();

    if (gasSensorHandler() == DUAL_SENSOR_ERROR)
    {
    	ledSetLED(LED4, LED_ON);
    	applicationSendEvent(EVT_ID_SENSOR_FAILED);

    	return ERROR_GENERAL;
    }

    applicationSendEvent(EVT_ID_INIT_READY);

    return ERROR_OK;
}

/**
 * @brief State handler for initialization state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onEntryPreOperational(State_t* pState, int32_t eventID)
{
	(void)pState;
	(void)eventID;

	ledSetLED(LED0, LED_OFF);

	return ERROR_OK;
}

/**
 * @brief State handler for pre-operational state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onStatePreOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

/**
 * @brief Entry handler for operational state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onEntryOperational(State_t* pState, int32_t eventID)
{
	(void)pState;
	(void)eventID;

	ledSetLED(LED0, LED_ON);
	ledSetLED(LED1, LED_OFF);

	return ERROR_OK;
}

/**
 * @brief State handler for operational state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onStateOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

/**
 * @brief State handler for emergency state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onStateEmergency(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

/**
 * @brief Exit handler for emergency state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onExitEmergency(State_t* pState, int32_t eventID)
{
	(void)pState;
	(void)eventID;

	emergencyTimer = HAL_GetTick();
	warningTimer = HAL_GetTick();

	return ERROR_OK;
}

/**
 * @brief Entry handler for failure state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onEntryFailure(State_t* pState, int32_t eventID)
{
	(void)pState;
	(void)eventID;

	ledSetLED(LED0, LED_OFF);
	ledSetLED(LED2, LED_ON);

	return ERROR_OK;
}

/**
 * @brief State handler for failure state
 *
 * @param pState Pointer to current state
 * @param eventID Current event ID
 *
 * @return Returns handler result
 */
static int32_t onStateFailure(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    return ERROR_OK;
}

/**
 * @brief Toggles dash symbol on left and right display
 *
 * @return Returns ERROR_OK or CURRENT_STATE_ERROR
 */
int32_t toggleDashSymbol()
{
	if(gStateTable.currentStateID == STATE_ID_OPERATIONAL) return CURRENT_STATE_ERROR;

	if (displayToggle == 0)
	{
		displayShowDigit(LEFT_DISPLAY, DASH_DIGIT);
		displayToggle = 1;
	}
	else
	{
		displayShowDigit(RIGHT_DISPLAY, DASH_DIGIT);
		displayToggle = 0;
	}

	return gStateTable.currentStateID;
}

/**
 * @brief Handles gas sensor related application processing
 *
 * @return Returns function result
 */
int32_t AppGasSensorHandler(void)
{
	int32_t sensorStatus = DUAL_SENSOR_OK;

	if(gStateTable.currentStateID == STATE_ID_OPERATIONAL)
	{
		sensorStatus = gasSensorHandler();
		ppmThresholdChecking();

		if(sensorStatus == DUAL_SENSOR_ERROR)
		{
			ledSetLED(LED4, LED_ON);
			applicationSendEvent(EVT_ID_SENSOR_FAILED);
		}
	}

	return ERROR_OK;
}

/**
 * @brief Handles emergency LED blinking
 *
 * @return Returns function result
 */
int32_t emergencyBlicking(){

	if (gStateTable.currentStateID == STATE_ID_EMERGENCY){
		ledToggleLED(LED1);
	}

	return ERROR_OK;
}

/**
 * @brief Checks gas sensor thresholds for warning and emergency behavior
 *
 * @return Returns function result
 */
int32_t ppmThresholdChecking(){

	uint32_t avrg = 0;

	now = HAL_GetTick();
	elapsed = 0u;

	getAvrg(&avrg);

	if (avrg > EMERGENCY_THRESHOLD){

		if (emergencyTimer == 0) emergencyTimer = now;

		elapsed = now - emergencyTimer;

		if (elapsed > EMERGENCY_TIME_ELAPSED){

			applicationSendEvent(EVT_ID_EMERGENCY_TRIGGERED);

			return DUAL_SENSOR_OK;
		}

	}
	else emergencyTimer = 0;

	if (avrg > WARNING_THRESHOLD){

			if (warningTimer == 0) warningTimer = now;

			if ((now - warningTimer) > WARNING_TIME_ELAPSED )
			{
				ledSetLED(LED1, LED_ON);

				return DUAL_SENSOR_OK;
			}
	}
	else
	{
		warningTimer = 0;
		ledSetLED(LED1, LED_OFF);
	}

	return DUAL_SENSOR_OK;
}

/**
 * @brief Reads the water sensor value and updates the display.
 *
 * @details If the system is in operational state, the water level is read
 *          from the sensor and shown on the 7-segment display.
 *
 * @return Returns SENSOR_OK.
 */
int32_t waterSensorHandler()
{
	if(gStateTable.currentStateID == STATE_ID_OPERATIONAL)
	{
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

