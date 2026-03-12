/******************************************************************************
 * @file AppTasks.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header File for the application tasks
 *
 *
 *****************************************************************************/
#ifndef _APPTASKS_H_
#define _APPTASKS_H_


/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/


/***** TYPES *****************************************************************/


/***** PROTOTYPES ************************************************************/

/**
 * @brief 10 ms application task
 *
 */
void taskApp10ms();

/**
 * @brief 50 ms application task
 *
 */
void taskApp50ms();

/**
 * @brief 250 ms application task
 *
 */
void taskApp250ms();


#endif /* SRC_APP_APPTASKS_H_ */
