/******************************************************************************
 * @file Scheduler.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header File for cooperative scheduler module
 *
 *
 *****************************************************************************/
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_


/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define SCHED_ERR_OK                0           //!< No error occured (Scheduler)
#define SCHED_ERR_INVALID_PTR       -1          //!< Invalid pointer (Scheduler)

/***** TYPES *****************************************************************/

/**
 * @brief Function pointer for reading the current HAL tick timer
 *
 * @details This provides the possibility to decouple the scheduler from a
 *          specific HAL implementation, but still makes it possible to
 *          measure time inside the scheduler.
 *
 */
typedef uint32_t (*GetHALTick)(void);

/**
 * @brief Function pointer for cyclic scheduler task functions
 *
 */
typedef void (*CyclicFunction)(void);

/**
 * @brief Scheduler object holding callback functions and task timestamps
 *
 */
typedef struct _Scheduler
{
    GetHALTick pGetHALTick;             //!< Function pointer for callback to read current HAL tick counter

    uint32_t halTick_10ms;              //!< Timestamp for last execution of 10ms task
    CyclicFunction pTask_10ms;          //!< Function pointer to 10ms cyclic task function

    uint32_t halTick_50ms;             //!< Timestamp for last execution of 50ms task
    CyclicFunction pTask_50ms;         //!< Function pointer to 50ms cyclic task function

    uint32_t halTick_250ms;             //!< Timestamp for last execution of 250ms task
    CyclicFunction pTask_250ms;         //!< Function pointer to 250ms cyclic task function

} Scheduler;


/***** PROTOTYPES ************************************************************/

/**
 * @brief Initializes the scheduler component
 *
 * @details Initializes the internal timestamp values of the scheduler.
 *
 * @remark This function does not initialize the function pointers in the
 *         scheduler struct.
 *
 * @param pScheduler Pointer to scheduler struct
 *
 * @return Returns SCHED_ERR_OK if no error occurred
 */
int32_t schedInitialize(Scheduler* pScheduler);

/**
 * @brief Cyclic function for the scheduler
 *
 * @details This function should be called in the system super loop. The
 *          scheduler takes care of the different task time slots.
 *
 * @param pScheduler Pointer to scheduler struct
 *
 * @return Returns SCHED_ERR_OK if no error occurred
 */
int32_t schedCycle(Scheduler* pScheduler);

#endif
