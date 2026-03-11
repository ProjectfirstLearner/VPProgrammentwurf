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
#include <stddef.h>


/*********GlobalObjects***************++/ */




/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/

/***PRIVATE FUNCTIONS***/

#define APP_ADC_MAX_VALUE       4095U
#define APP_ADC_REFERENCE_UV    3300000U

GasSensor gGasSensor1;

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


	int32_t adcValue = adcReadChannel(ADC_INPUT0);
	int32_t sensorVoltageCheck = gasSensorSetSensorVoltage(&gGasSensor1, adcValue);
    int32_t gasValue1 = gasSensorGetSensorValue(&gGasSensor1);

    outputLogf("Gas1Voltage: %ld  Gas1ppm: %ld\n\r", adcValue, gasValue1);

    //gasValue1 = gasSensorGetSensorValue(&gGasSensor1);

}


/***** PRIVATE FUNCTIONS *****************************************************/




