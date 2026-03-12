/*
 * GasSensor.c
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */
#include "WaterSensor.h"
#include "stddef.h"

// in ppm
#define MIN_SENSOR_VALUE        50
#define MAX_SENSOR_VALUE        1000

#define MIN_SENSOR_VOLTAGE      500000U
#define MAX_SENSOR_VOLTAGE      2500000U

#define CONVERSION_FACTOR		2105


WaterSensor waterSensor;

int32_t WaterSensorInitialize()
{

    waterSensor.sensorVoltage = 0U;

    return SENSOR_OK;
}

//sensorVoltage is in micro
int32_t WaterSensorSetSensorVoltage(uint32_t sensorVoltage)
{
    //prüfen ob zwischen 0,5 und 2,5V

    if (sensorVoltage < MIN_SENSOR_VOLTAGE || sensorVoltage >MAX_SENSOR_VOLTAGE) return SENSOR_VOLTAGE_INVALID;

    waterSensor.sensorVoltage = sensorVoltage;

    return SENSOR_OK;
}

int32_t WaterSensorGetSensorValue(uint32_t *Watervalue)
{

	int32_t differenceVoltage = 0u;

    //checking again, could theoretically be overidden
	if (waterSensor.sensorVoltage < MIN_SENSOR_VOLTAGE || waterSensor.sensorVoltage >MAX_SENSOR_VOLTAGE) return SENSOR_VOLTAGE_INVALID;

    //using MIN_SENSOR_VOLTAGE as Offset
    differenceVoltage = waterSensor.sensorVoltage - MIN_SENSOR_VOLTAGE;

    *Watervalue = MIN_SENSOR_VALUE + (int32_t)(differenceVoltage / CONVERSION_FACTOR);

    //same here but for ppm
    if ((*Watervalue < MIN_SENSOR_VALUE) || (*Watervalue > MAX_SENSOR_VALUE))
    {
        return SENSOR_VALUE_INVALID;
    }

    return SENSOR_OK;
}







