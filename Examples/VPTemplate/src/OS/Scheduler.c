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
#include "Scheduler.h"
#include <stddef.h>

/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/** @brief Number of HAL ticks corresponding to 10 ms. */
#define HAL_TICK_VALUE_10MS     10U

/** @brief Number of HAL ticks corresponding to 50 ms. */
#define HAL_TICK_VALUE_50MS     50U

/** @brief Number of HAL ticks corresponding to 250 ms. */
#define HAL_TICK_VALUE_250MS    250U

/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/

/**
 * @brief Calculates the elapsed time between two HAL tick values.
 *
 * @details
 * The calculation is based on unsigned subtraction and therefore also works
 * correctly when the HAL tick counter wraps around.
 *
 * @param[in] savedTimeStamp
 * Previously stored tick value.
 *
 * @param[in] currentTime
 * Current HAL tick value.
 *
 * @return Elapsed time in ticks.
 */
static uint32_t schedulerGetElapseTime(uint32_t savedTimeStamp, uint32_t currentTime);

/***** PUBLIC FUNCTIONS ******************************************************/

/**
 * @brief Initializes the scheduler instance.
 *
 * @details
 * Resets all internal timestamp values of the scheduler.
 *
 * @param[in,out] pScheduler
 * Pointer to the scheduler instance to initialize.
 *
 * @retval SCHED_ERR_OK
 * Scheduler was initialized successfully.
 *
 * @retval SCHED_ERR_INVALID_PTR
 * `pScheduler` is `NULL`.
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
 * @brief Executes one scheduler cycle.
 *
 * @details
 * Checks all configured scheduler time slots and executes the corresponding
 * task function if the required period has elapsed since the last execution.
 *
 * The following cyclic tasks are supported:
 * - 10 ms task
 * - 50 ms task
 * - 250 ms task
 *
 * @param[in,out] pScheduler
 * Pointer to the scheduler instance.
 *
 * @retval SCHED_ERR_OK
 * Scheduler cycle completed successfully.
 *
 * @retval SCHED_ERR_INVALID_PTR
 * `pScheduler` is `NULL` or a required task pointer is `NULL`.
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
 * @brief Calculates elapsed scheduler time in HAL ticks.
 *
 * @param[in] savedTimeStamp
 * Previously stored timestamp.
 *
 * @param[in] currentTime
 * Current HAL tick value.
 *
 * @return Difference between current and saved timestamp in ticks.
 */

//inline so that compiler dosnt jump but writes code directly
static inline uint32_t schedulerGetElapseTime(uint32_t savedTimeStamp, uint32_t currentTime)
{
	uint32_t dt = currentTime - savedTimeStamp;
	return dt;
}
