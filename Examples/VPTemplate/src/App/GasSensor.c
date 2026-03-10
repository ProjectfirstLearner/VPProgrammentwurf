#include "GasSensor.h"
#include "stddef.h"
// in ppm
#define MIN_SENSOR_VALUE        200
#define MAX_SENSOR_VALUE        10000

#define MIN_SENSOR_VOLTAGE      500000U
#define MAX_SENSOR_VOLTAGE      2500000U

int32_t gasSensorInitialize(GasSensor* pSensor, uint32_t convFactor)
{
    if (pSensor == NULL)
    {
        return SENSOR_INVALID_PTR;
    }

    pSensor->sensorVoltage = 0U;
    pSensor->conversionFactor = convFactor;

    return SENSOR_OK;
}

int32_t gasSensorSetSensorVoltage(GasSensor* pSensor, uint32_t sensorVoltage)
{
    if (pSensor == NULL)
    {
        return SENSOR_INVALID_PTR;
    }

    pSensor->sensorVoltage = sensorVoltage;

    return SENSOR_OK;
}

int32_t gasSensorGetSensorValue(GasSensor* pSensor)
{
    uint32_t deltaVoltageUv = 0U;
    int32_t value = 0;

    if (pSensor == NULL)
    {
        return SENSOR_INVALID_PTR;
    }

    if ((pSensor->sensorVoltage < MIN_SENSOR_VOLTAGE) ||
        (pSensor->sensorVoltage > MAX_SENSOR_VOLTAGE))
    {
        return SENSOR_VALUE_INVALID;
    }

    deltaVoltageUv = pSensor->sensorVoltage - MIN_SENSOR_VOLTAGE;

    value = MIN_SENSOR_VALUE + (int32_t)(deltaVoltageUv / pSensor->conversionFactor);

    if ((value < MIN_SENSOR_VALUE) || (value > MAX_SENSOR_VALUE))
    {
        return SENSOR_VALUE_INVALID;
    }

    return value;
}

/*
int32_t gasSensorGetSensorVoltage(GasSensor* pSensor)
{
    if (pSensor == NULL)
    {
        return SENSOR_INVALID_PTR;
    }

    if ((pSensor->sensorVoltage < MIN_SENSOR_VOLTAGE) ||
        (pSensor->sensorVoltage > MAX_SENSOR_VOLTAGE))
    {
        return SENSOR_VOLTAGE_INVALID;
    }

    return (int32_t)pSensor->sensorVoltage;
}


*/






