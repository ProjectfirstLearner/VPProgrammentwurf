/******************************************************************************
 * @file Scheduler.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of a cooperative scheduler with predefined cyclic task
 * slots.
 *
 * @details
 * This module implements a simple cooperative scheduler based on the HAL system
 * tick. It supports cyclic execution of task functions in fixed time slots:
 * 10 ms, 50 ms, and 250 ms.
 *
 * The scheduler compares the current HAL tick value with the stored timestamps
 * of the individual task slots. If the configured period has elapsed, the
 * corresponding task function is executed.
 *
 * @note
 * The scheduler is cooperative. Tasks are executed sequentially and must not
 * block for a long time.
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include <stddef.h>

#include "Scheduler.h"

/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/** @brief Number of HAL ticks corresponding to 10, 50, 250 ms. */
#define HAL_TICK_VALUE_10MS     10U
#define HAL_TICK_VALUE_50MS     50U
#define HAL_TICK_VALUE_250MS    250U

/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static inline uint32_t schedulerGetElapseTime(uint32_t savedTimeStamp, uint32_t currentTime);


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/

/**
 * @brief Initializes the scheduler instance
 *
 * @param pScheduler Pointer to scheduler instance
 *
 * @return Returns SCHED_ERR_OK if no error occurred
 */
int32_t schedInitialize(Scheduler* pScheduler)
{
	if(pScheduler == NULL)
	{
		return SCHED_ERR_INVALID_PTR;
	}


	pScheduler->halTick_10ms = 0U;
	pScheduler->halTick_50ms = 0U;
	pScheduler->halTick_250ms = 0U;

	pScheduler->pGetHALTick = NULL;
	pScheduler->pTask_10ms = NULL;
	pScheduler->pTask_50ms = NULL;
	pScheduler->pTask_250ms = NULL;

    return SCHED_ERR_OK;
}

/**
 * @brief Executes one scheduler cycle
 *
 * @param pScheduler Pointer to scheduler instance
 *
 * @return Returns SCHED_ERR_OK if no error occurred
 */
int32_t schedCycle(Scheduler* pScheduler)
{
	uint32_t timeElapsed = 0U;
	uint32_t actualTick= 0U;


	if(pScheduler == NULL)
	{
		return SCHED_ERR_INVALID_PTR;
	}

	if (pScheduler->pGetHALTick == NULL)
	{
	    return SCHED_ERR_INVALID_PTR;
	}

	actualTick = pScheduler->pGetHALTick();

	timeElapsed = schedulerGetElapseTime(pScheduler->halTick_10ms, actualTick);
	if(timeElapsed >= HAL_TICK_VALUE_10MS)
	{
		pScheduler->halTick_10ms = actualTick;
		if(pScheduler->pTask_10ms != NULL)
		{
			pScheduler->pTask_10ms();
		}else
		{
			return SCHED_ERR_INVALID_PTR;

		}
	}

	timeElapsed = schedulerGetElapseTime(pScheduler->halTick_50ms, actualTick);
	if(timeElapsed >= HAL_TICK_VALUE_50MS)
	{
		pScheduler->halTick_50ms = actualTick;
		if(pScheduler->pTask_50ms != NULL)
		{
			pScheduler->pTask_50ms();
		}else
		{
			return SCHED_ERR_INVALID_PTR;

		}
	}

	timeElapsed = schedulerGetElapseTime(pScheduler->halTick_250ms, actualTick);
	if(timeElapsed >= HAL_TICK_VALUE_250MS)
	{
		pScheduler->halTick_250ms = actualTick;
		if(pScheduler->pTask_250ms != NULL)
		{
			pScheduler->pTask_250ms();
		}else
		{
			return SCHED_ERR_INVALID_PTR;

		}
	}

	return SCHED_ERR_OK;
}


/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Calculates elapsed scheduler time in HAL ticks
 *
 * @param savedTimeStamp Previously stored timestamp
 * @param currentTime Current HAL tick value
 *
 * @return Returns difference between current and saved timestamp in ticks
 */
static inline uint32_t schedulerGetElapseTime(uint32_t savedTimeStamp, uint32_t currentTime)
{
	uint32_t dt = currentTime - savedTimeStamp;

	return dt;
}
