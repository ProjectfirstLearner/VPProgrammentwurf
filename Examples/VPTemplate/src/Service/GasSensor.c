/*
 * GasSensor.c
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */

#include "GasSensor.h"

#include "stddef.h"

/* in ppm */
#define MIN_SENSOR_VALUE        200
#define MAX_SENSOR_VALUE        10000

#define MIN_SENSOR_VOLTAGE      500000U
#define MAX_SENSOR_VOLTAGE      2500000U

#define CONVERSION_FACTOR		204u

int32_t gasSensorInitialize(GasSensor* pSensor)
{
    if (pSensor == NULL)
    {
        return SENSOR_INVALID_PTR;
    }

    pSensor->sensorVoltage = 0U;

    return SENSOR_OK;
}

/* sensorVoltage is in micro */
int32_t gasSensorSetSensorVoltage(GasSensor* pSensor, EMAFilterData_t* pEMA, uint32_t sensorVoltage)
{
	/* Check if GasSensor pointer is valid */
    if (pSensor == NULL) return SENSOR_INVALID_PTR;

    /* Check if voltage is between 0.5 V and 2.5 V */
    if (sensorVoltage < MIN_SENSOR_VOLTAGE || sensorVoltage >MAX_SENSOR_VOLTAGE) return SENSOR_VOLTAGE_INVALID;

    int32_t filtered = filterEMA(pEMA, sensorVoltage);

    pSensor->sensorVoltage = sensorVoltage;

    return SENSOR_OK;
}

int32_t gasSensorGetSensorValue(GasSensor* pSensor, uint32_t *gasvalue)
{

	int32_t differenceVoltage = 0u;

	if (gasvalue == NULL)
	    {
	        return SENSOR_INVALID_PTR;
	    }

    if (pSensor == NULL)

    {
        return SENSOR_INVALID_PTR;
    }

    /* Checking again, could theoretically be overridden */
    if (pSensor->sensorVoltage < MIN_SENSOR_VOLTAGE || pSensor->sensorVoltage >MAX_SENSOR_VOLTAGE) return SENSOR_VOLTAGE_INVALID;

    /* Using MIN_SENSOR_VOLTAGE as offset */
    differenceVoltage = pSensor->sensorVoltage - MIN_SENSOR_VOLTAGE;

    *gasvalue = MIN_SENSOR_VALUE + (int32_t)(differenceVoltage / CONVERSION_FACTOR);

    /* Same here but for ppm */
    if ((*gasvalue < MIN_SENSOR_VALUE) || (*gasvalue > MAX_SENSOR_VALUE))
    {
        return SENSOR_VALUE_INVALID;
    }

    return SENSOR_OK;
}







