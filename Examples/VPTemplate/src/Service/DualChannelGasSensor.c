/*
 * DualChannelGasSensor.c
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */

#include "DualChannelGasSensor.h"
#include "ADCModule.h"
#include "GasSensor.h"
#include "Log/LogOutput.h"
#include "Filter/Filter.h"

#include <stdlib.h>

/***** PRIVATE MACROS ********************************************************/
#define SCALING_FACTOR			1000
#define ALPHA					200
#define COUNT_SENSORS			2
#define ALLOWED_DIFFERENCE		50
#define PERCENTAGE_FACTOR		100

/***** PRIVATE VARIABLES *****************************************************/
EMAFilterData_t pEMA1;
EMAFilterData_t pEMA2;
GasSensor gGasSensor1;
GasSensor gGasSensor2;

static uint32_t adcValue1 = 0;
static uint32_t adcValue2 = 0;

static uint32_t gasValue1 = 0;
static uint32_t gasValue2 = 0;
static uint32_t avrg = 0;

/***** PUBLIC FUNCTIONS ******************************************************/

int32_t DualChannelInit(void)
{
	gasSensorInitialize(&gGasSensor1);
	gasSensorInitialize(&gGasSensor2);

	filterInitEMA(&pEMA1, SCALING_FACTOR, ALPHA, false);
	filterInitEMA(&pEMA2, SCALING_FACTOR, ALPHA, false);

	return DUAL_SENSOR_OK;
};


int32_t DualChannelSetVoltage(void)
{
	adcValue1 = adcReadChannel(ADC_INPUT0);
	int32_t sensorVoltageCheck1 = gasSensorSetSensorVoltage(&gGasSensor1, &pEMA1, adcValue1);

	if (sensorVoltageCheck1 != SENSOR_OK) return DUAL_SENSOR_ERROR;

	adcValue2 = adcReadChannel(ADC_INPUT1);
	int32_t sensorVoltageCheck2 = gasSensorSetSensorVoltage(&gGasSensor2, &pEMA2, adcValue2);

	if (sensorVoltageCheck2 != SENSOR_OK) return DUAL_SENSOR_ERROR;

    return DUAL_SENSOR_OK;
};

int32_t DualChannelVoltageAverage(void)
{
	int32_t sensorValueCheck1 = gasSensorGetSensorValue(&gGasSensor1, &gasValue1);

	if (sensorValueCheck1 != SENSOR_OK) return DUAL_SENSOR_ERROR;

	int32_t sensorValueCheck2 = gasSensorGetSensorValue(&gGasSensor2, &gasValue2);

	if (sensorValueCheck2 != SENSOR_OK) return DUAL_SENSOR_ERROR;

	avrg = (gasValue1 + gasValue2)/COUNT_SENSORS;

	return DUAL_SENSOR_OK;
};

int32_t DualChannelUpdate()
{
	int32_t difference = abs((gasValue1- gasValue2));

	int32_t smallerValue = 0;

	if (gasValue1 > gasValue2) smallerValue = gasValue2;
	else smallerValue = gasValue1;

	int32_t proofdifference =  smallerValue * ALLOWED_DIFFERENCE / PERCENTAGE_FACTOR;

	if (difference > proofdifference) return DUAL_SENSOR_ERROR;

	return DUAL_SENSOR_OK;
};

int32_t gasSensorHandler(void)
{
	DualChannelSetVoltage();
	DualChannelVoltageAverage();
	DualChannelUpdate();

	if((DualChannelSetVoltage() == DUAL_SENSOR_ERROR) || (DualChannelVoltageAverage() == DUAL_SENSOR_ERROR) || (DualChannelUpdate() == DUAL_SENSOR_ERROR))
		return DUAL_SENSOR_ERROR;

	return DUAL_SENSOR_OK;
}

int32_t getAvrg(uint32_t *avrgValue){

	*avrgValue = avrg;
	return DUAL_SENSOR_OK;
}
