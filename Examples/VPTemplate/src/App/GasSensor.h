#ifndef _GAS_SENSOR_H_
#define _GAS_SENSOR_H_

#include <stdint.h>

/***** MACROS ****************************************************************/
#define SENSOR_OK               0
#define SENSOR_INVALID_PTR     -1
#define SENSOR_VALUE_INVALID   -2
#define SENSOR_VOLTAGE_INVALID -3

/***** TYPES *****************************************************************/
typedef struct
{
    uint32_t sensorVoltage;
    uint32_t conversionFactor;
} GasSensor;

/***** PROTOTYPES ************************************************************/
int32_t gasSensorInitialize(GasSensor* pSensor, uint32_t convFactor);
int32_t gasSensorSetSensorVoltage(GasSensor* pSensor, uint32_t sensorVoltage);
int32_t gasSensorGetSensorValue(GasSensor* pSensor);
int32_t gasSensorGetSensorVoltage(GasSensor* pSensor);

#endif
