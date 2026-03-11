/*
 * GasSensor.h
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */
#ifndef _GAS_SENSOR_H_
#define _GAS_SENSOR_H_

#include <stdint.h>

/***** MACROS ****************************************************************/
#define SENSOR_OK               0u
#define SENSOR_INVALID_PTR     -1
#define SENSOR_VALUE_INVALID   -2
#define SENSOR_VOLTAGE_INVALID -3

/***** TYPES *****************************************************************/
typedef struct
{
    uint32_t sensorVoltage;
} GasSensor;

/***** PROTOTYPES ************************************************************/
//initializing
int32_t gasSensorInitialize(GasSensor* pSensor);
//checking if in valid area, also filters
int32_t gasSensorSetSensorVoltage(GasSensor* pSensor, uint32_t sensorVoltage);
//getter for sensor data in ppm
int32_t gasSensorGetSensorValue(GasSensor* pSensor);

#endif
