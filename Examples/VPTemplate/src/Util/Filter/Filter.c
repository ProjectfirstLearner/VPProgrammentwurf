/******************************************************************************
 * @file Filter.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation file for Filter library
 *
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "Filter.h"

#include <stddef.h>
/***** PRIVATE CONSTANTS *****************************************************/

/***** PRIVATE MACROS ********************************************************/

/***** PRIVATE TYPES *********************************************************/

/***** PRIVATE PROTOTYPES ****************************************************/

/***** PRIVATE VARIABLES *****************************************************/

/***** PUBLIC FUNCTIONS ******************************************************/

int32_t filterInitEMA(EMAFilterData_t* pEMA, int32_t scalingFactor, int32_t alpha, bool resetFilter)
{
    if (pEMA == NULL)
    {
        return FILTER_ERR_INVALID_PTR;
    }

    if ((scalingFactor <= 0) || (alpha <= 0) || (alpha > scalingFactor))
    {
        return FILTER_ERR_INVALID_PARAM;
    }

    pEMA->scalingFactor = scalingFactor;
    pEMA->alpha = alpha;

    if (resetFilter == true)
    {
        pEMA->firstValueAvailable = false;
        pEMA->previousValue = 0;
    }

    return FILTER_ERR_OK;
}

int32_t filterResetEMA(EMAFilterData_t* pEMA)
{
    if (pEMA == NULL)
    {
        return FILTER_ERR_INVALID_PTR;
    }

    pEMA->firstValueAvailable = false;
    pEMA->previousValue = 0;

    return FILTER_ERR_OK;
}

int32_t filterEMA(EMAFilterData_t* pEMA, int32_t sensorValue)
{
    int32_t filteredValue = 0;
    int64_t weightedNewValue = 0;
    int64_t weightedOldValue = 0;

    if (pEMA == NULL)
    {
        return FILTER_ERR_INVALID_PTR;
    }

    if ((pEMA->scalingFactor <= 0) || (pEMA->alpha <= 0) || (pEMA->alpha > pEMA->scalingFactor))
    {
        return FILTER_ERR_INVALID_PARAM;
    }

    if (pEMA->firstValueAvailable == false)
    {
        pEMA->previousValue = sensorValue;
        pEMA->firstValueAvailable = true;
        return FILTER_ERR_OK;
    }

    weightedNewValue = (int64_t)pEMA->alpha * (int64_t)sensorValue;
    weightedOldValue = (int64_t)(pEMA->scalingFactor - pEMA->alpha) * (int64_t)pEMA->previousValue;

    filteredValue = (int32_t)((weightedNewValue + weightedOldValue) / pEMA->scalingFactor);

    pEMA->previousValue = filteredValue;

    return FILTER_ERR_OK;
}
