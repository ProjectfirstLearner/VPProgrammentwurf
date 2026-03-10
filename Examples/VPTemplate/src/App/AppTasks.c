/******************************************************************************
 * @file AppTasks.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation File for the application tasks
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "Scheduler.h"
#include "AppTasks.h"
#include "Application.h"
#include "GasSensor.h"

#include "ADCModule.h"
#include "Util/Log/printf.h"
#include "Util/Log/LogOutput.h"

/*temporary*/

#include <stdint.h>

/*********GlobalObjects***************++/ */

extern GasSensor gGasSensor1;
extern GasSensor gGasSensor2;



/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/

/***PRIVATE FUNCTIONS***/

#define APP_ADC_MAX_VALUE       4095U
#define APP_ADC_REFERENCE_UV    3300000U

static uint32_t convertAdcRawToMicroVolt(uint32_t adcRaw)
{
    uint64_t scaledValue = 0U;

    scaledValue = ((uint64_t)adcRaw * (uint64_t)APP_ADC_REFERENCE_UV) / (uint64_t)APP_ADC_MAX_VALUE;

    return (uint32_t)scaledValue;
}

/***** PUBLIC FUNCTIONS ******************************************************/

void taskApp10ms()
{

}


void taskApp50ms()
{
	applicationRun();
}

void taskApp250ms()
{
    int32_t adcValue = 0;
    uint32_t sensorVoltageUv = 0U;
    int32_t gasValue1 = 0;
    int32_t gasValue2 = 0;

    adcValue = adcReadChannel(ADC_INPUT0);
    sensorVoltageUv = convertAdcRawToMicroVolt((uint32_t)adcValue);
    gasSensorSetSensorVoltage(&gGasSensor1, sensorVoltageUv);
    gasValue1 = gasSensorGetSensorValue(&gGasSensor1);

    adcValue = adcReadChannel(ADC_INPUT1);
    sensorVoltageUv = convertAdcRawToMicroVolt((uint32_t)adcValue);
    gasSensorSetSensorVoltage(&gGasSensor2, sensorVoltageUv);
    gasValue2 = gasSensorGetSensorValue(&gGasSensor2);

    outputLogf("Gas1: %ld  Gas2: %ld\n\r", gasValue1, gasValue2);
}


/***** PRIVATE FUNCTIONS *****************************************************/




