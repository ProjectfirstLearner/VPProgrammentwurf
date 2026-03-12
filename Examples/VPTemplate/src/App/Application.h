/******************************************************************************
 * @file Application.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header file for main application (state machine)
 *
 *
 *****************************************************************************/
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>


/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/


/***** TYPES *****************************************************************/

/**
 * @brief Enumeration of application state IDs
 *
 */
typedef enum
{
    STATE_ID_INITIALIZATION = 0,
    STATE_ID_PRE_OPERATIONAL = 1,
    STATE_ID_OPERATIONAL = 2,
    STATE_ID_EMERGENCY,
    STATE_ID_TEST_MODE,
    STATE_ID_FAILURE
} ApplicationStateID_t;

/**
 * @brief Enumeration of application event IDs
 *
 */
typedef enum
{
	EVT_ID_NO_ID = 0,
    EVT_ID_INIT_READY = 1,				/* starting at 1, because 0 is defined as "STT_NONE_EVENT",
    										avoiding a misleading event if stateTableSendEvent() sends 0 */
    EVT_ID_SENSOR_FAILED,
    EVT_ID_SWITCH_TO_OPERATIONAL,
    EVT_ID_SWITCH_TO_PRE_OPERATIONAL,
    EVT_ID_EMERGENCY_TRIGGERED,
    EVT_ID_ALARM_RESET,
    EVT_ID_TEST_MODE_TRIGGERED,
    EVT_ID_STACK_CORRUPTION
} ApplicationEventID_t;

/***** PROTOTYPES ************************************************************/

/**
 * @brief Initializes application state list, transition table and start state
 *
 * @return Returns initialization result
 */
int32_t applicationInitialize(void);

/**
 * @brief Runs the application state machine cyclically
 *
 * @return Returns state machine execution result
 */
int32_t applicationRun(void);

/**
 * @brief Sends an event to the application state machine
 *
 * @param eventID Event ID to send
 *
 * @return Returns result of event handling
 */
int32_t applicationSendEvent(int32_t eventID);

/**
 * @brief Handles gas sensor related application logic
 *
 * @return Returns function result
 */
int32_t AppGasSensorHandler(void);

/**
 * @brief Returns current active application state
 *
 * @return Returns current state ID or error value
 */
int32_t applicationGetCurrentState(void);

/**
 * @brief Handles emergency blinking indication
 *
 * @return Returns function result
 */
int32_t emergencyBlicking(void);

/**
 * @brief Handles water sensor related logic
 *
 * @return Returns function result
 */
int32_t waterSensorHandler(void);

/**
 * @brief Toggles dash symbol on display
 *
 * @return Returns current state ID or error value
 */
int32_t toggleDashSymbol(void);


#endif
