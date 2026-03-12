/*
 * WaterSensor.h
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

/**
 * @brief Initializes the water sensor module.
 *
 * @return SENSOR_OK
 */
int32_t WaterSensorInitialize(void);

/**
 * @brief Sets the sensor voltage if it is within the valid range.
 *
 * @param sensorVoltage Sensor voltage in microvolts.
 *
 * @return SENSOR_OK on success, SENSOR_VOLTAGE_INVALID on invalid voltage
 */
int32_t WaterSensorSetSensorVoltage(uint32_t sensorVoltage);

/**
 * @brief Returns the calculated sensor value.
 *
 * @param value Pointer to output value.
 *
 * @return SENSOR_OK on success, error code otherwise
 */
int32_t WaterSensorGetSensorValue(uint32_t *value);

#endif
