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
typedef enum
{
    STATE_ID_INITIALIZATION = 0,
    STATE_ID_PRE_OPERATIONAL = 1,
    STATE_ID_OPERATIONAL = 2,
    STATE_ID_EMERGENCY,
    STATE_ID_TEST_MODE,
    STATE_ID_FAILURE
} ApplicationStateID_t;

typedef enum
{
	EVT_ID_NO_ID = 0,
    EVT_ID_INIT_READY = 1,				/*starting at 1, becasue 0 is defined as "STT_NONE_EVENT",
    										dodging a mislead event if stateTableSendEvent() sends 0*/
    EVT_ID_SENSOR_FAILED,
    EVT_ID_SWITCH_TO_OPERATIONAL,
    EVT_ID_SWITCH_TO_PRE_OPERATIONAL,
    EVT_ID_EMERGENCY_TRIGGERED,
    EVT_ID_ALARM_RESET,
    EVT_ID_TEST_MODE_TRIGGERED,
    EVT_ID_STACK_CORRUPTION
} ApplicationEventID_t;

/***** PROTOTYPES ************************************************************/

//function to asign state list, transition table, and start state
int32_t applicationInitialize(void);

//gets called every 50ms
int32_t applicationRun(void);

//function used to send events to state machine
int32_t applicationSendEvent(int32_t eventID);

int32_t AppGasSensorHandler(void);

int32_t applicationGetCurrentState();

#endif
