/*
 * GasSensor.h
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */
#ifndef _WATER_SENSOR_H_
#define _WATER_SENSOR_H_

#include <stdint.h>

/***** MACROS ****************************************************************/
#define SENSOR_OK               0u
#define SENSOR_VALUE_INVALID   -1
#define SENSOR_VOLTAGE_INVALID -2

/***** TYPES *****************************************************************/
typedef struct
{
    uint32_t sensorVoltage;
} WaterSensor;

/***** PROTOTYPES ************************************************************/
//initializing
int32_t WaterSensorInitialize();
//checking if in valid area, also filters
int32_t WaterSensorSetSensorVoltage(uint32_t sensorVoltage);
//getter for sensor data in ppm
int32_t WaterSensorGetSensorValue(uint32_t *value);

#endif
