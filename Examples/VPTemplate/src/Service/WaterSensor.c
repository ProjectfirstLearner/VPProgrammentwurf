/*
 * WaterSensor.c
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */

#include "WaterSensor.h"

/***** PRIVATE MACROS ********************************************************/

/* Sensor value in ppm */
#define MIN_SENSOR_VALUE        50
#define MAX_SENSOR_VALUE        1000

#define MIN_SENSOR_VOLTAGE      500000U
#define MAX_SENSOR_VOLTAGE      2500000U

#define CONVERSION_FACTOR		2105

/***** PRIVATE VARIABLES *****************************************************/

WaterSensor waterSensor;

/***** PUBLIC FUNCTIONS ******************************************************/

int32_t WaterSensorInitialize(void)
{

    waterSensor.sensorVoltage = 0u;

    return SENSOR_OK;
}

/* sensorVoltage is in microvolts */
int32_t WaterSensorSetSensorVoltage(uint32_t sensorVoltage)
{
	/* Check if voltage is between 0.5 V and 2.5 V */
    if (sensorVoltage < MIN_SENSOR_VOLTAGE || sensorVoltage >MAX_SENSOR_VOLTAGE) return SENSOR_VOLTAGE_INVALID;

    waterSensor.sensorVoltage = sensorVoltage;

    return SENSOR_OK;
}

int32_t WaterSensorGetSensorValue(uint32_t *Watervalue)
{
	int32_t differenceVoltage = 0u;

	/* Check again in case value was overwritten */
	if (waterSensor.sensorVoltage < MIN_SENSOR_VOLTAGE || waterSensor.sensorVoltage >MAX_SENSOR_VOLTAGE) return SENSOR_VOLTAGE_INVALID;

	/* Use MIN_SENSOR_VOLTAGE as offset */
    differenceVoltage = waterSensor.sensorVoltage - MIN_SENSOR_VOLTAGE;

    *Watervalue = MIN_SENSOR_VALUE + (int32_t)(differenceVoltage / CONVERSION_FACTOR);

    /* Check calculated ppm range */
    if ((*Watervalue < MIN_SENSOR_VALUE) || (*Watervalue > MAX_SENSOR_VALUE))
    {
        return SENSOR_VALUE_INVALID;
    }

    return SENSOR_OK;
}
