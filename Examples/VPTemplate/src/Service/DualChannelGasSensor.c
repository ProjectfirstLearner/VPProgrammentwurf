/*
 * DualGasSensor.c
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */
#include "DualChannelGasSensor.h"
#include "GasSensor.h"
#include "Filter/Filter.h"
#include "ADCModule.h"
#include "Log/LogOutput.h"
#include "stm32g4xx_hal.h"
#include "Application.h"
#include "LEDModule.h"
#include <stdlib.h>


#define SCALING_FACTOR			1000
#define ALPHA					200
#define COUNT_SENSORS			2
#define ALLOWED_DIFFERENCE		30
#define PERCENTAGE_FACTOR		100

#define WARNING_THRESHOLD		3000
#define EMERGENCY_THRESHOLD		5000

#define WARNING_TIME_ELAPSED	5000
#define EMERGENCY_TIME_ELAPSED	3000


EMAFilterData_t pEMA1;
EMAFilterData_t pEMA2;
GasSensor gGasSensor1;
GasSensor gGasSensor2;

static uint32_t adcValue1 = 0;
static uint32_t adcValue2 = 0;

static uint32_t gasValue1 = 0;
static uint32_t gasValue2 = 0;
static uint32_t avrg = 0;


static int32_t emergencyTimer = 0;
static int32_t warningTimer = 0;


int32_t DualChannelInit(){

	gasSensorInitialize(&gGasSensor1);
	gasSensorInitialize(&gGasSensor2);

	filterInitEMA(&pEMA1, SCALING_FACTOR, ALPHA, false);
	filterInitEMA(&pEMA2, SCALING_FACTOR, ALPHA, false);

	return DUAL_SENSOR_OK;
};


int32_t DualChannelSetVoltage(){


	adcValue1 = adcReadChannel(ADC_INPUT0);
	int32_t sensorVoltageCheck1 = gasSensorSetSensorVoltage(&gGasSensor1, &pEMA1, adcValue1);

	if (sensorVoltageCheck1 != SENSOR_OK) return DUAL_SENSOR_ERROR;



	adcValue2 = adcReadChannel(ADC_INPUT1);
	int32_t sensorVoltageCheck2 = gasSensorSetSensorVoltage(&gGasSensor2, &pEMA2, adcValue2);

	if (sensorVoltageCheck2 != SENSOR_OK) return DUAL_SENSOR_ERROR;


    return DUAL_SENSOR_OK;

};

int32_t DualChannelVoltageAverage(){


	int32_t sensorValueCheck1 = gasSensorGetSensorValue(&gGasSensor1, &gasValue1);

	if (sensorValueCheck1 != SENSOR_OK) return DUAL_SENSOR_ERROR;


	outputLogf("Gas1Voltage: %ld  Gas1ppm: %ld\n\r", adcValue1, gasValue1);

	int32_t sensorValueCheck2 = gasSensorGetSensorValue(&gGasSensor2, &gasValue2);

	if (sensorValueCheck2 != SENSOR_OK) return DUAL_SENSOR_ERROR;


	avrg = (gasValue1 + gasValue2)/COUNT_SENSORS;

	//outputLogf("Gas2Voltage: %ld  Gas2ppm: %ld\n\r", adcValue2, gasValue2);

	return DUAL_SENSOR_OK;
};

int32_t DualChannelUpdate(){

	int32_t difference = abs((gasValue1- gasValue2));

	int32_t smallerValue = 0;


	if (gasValue1 > gasValue2){
		smallerValue = gasValue2;
	}
	else{
		smallerValue = gasValue1;
	}


	int32_t proofdifference =  smallerValue * ALLOWED_DIFFERENCE / PERCENTAGE_FACTOR;

	if (difference > proofdifference)
	{
	    return DUAL_SENSOR_ERROR;
	}

	return DUAL_SENSOR_OK;

};



int32_t ppmThresholdChecking(){

	int32_t now = HAL_GetTick();


	if (avrg > EMERGENCY_THRESHOLD){

		if (emergencyTimer ==0){

			emergencyTimer = now;

		}

		uint32_t elapsed = now -emergencyTimer;

		if (elapsed > EMERGENCY_TIME_ELAPSED ){

			applicationSendEvent(EVT_ID_EMERGENCY_TRIGGERED);

			return DUAL_SENSOR_OK;
		}



	}
	else {
		emergencyTimer = 0;
	}
	if (avrg > WARNING_THRESHOLD){

			if (warningTimer ==0){

				warningTimer = now;

			}

			if ((now-warningTimer) > WARNING_TIME_ELAPSED )
			{
				ledSetLED(LED4, LED_ON);
				return DUAL_SENSOR_OK;
			}
		}
		else {
			warningTimer = 0;
		}
	return DUAL_SENSOR_OK;

}




int32_t gasSensorHandler()
{

	DualChannelSetVoltage();
	DualChannelVoltageAverage();
	DualChannelUpdate();
	ppmThresholdChecking();

	return DUAL_SENSOR_OK;
}






