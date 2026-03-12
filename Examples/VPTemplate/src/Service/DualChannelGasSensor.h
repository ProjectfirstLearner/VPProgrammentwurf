/*
 * DualGasSensor.h
 *
 *  Created on: Mar 11, 2026
 *      Author: kali
 */

#ifndef SRC_SERVICE_DUALCHANNELGASSENSOR_H_
#define SRC_SERVICE_DUALCHANNELGASSENSOR_H_



#include <stdint.h>

#define DUAL_SENSOR_OK               0u
#define DUAL_SENSOR_INVALID_PTR     -1
#define DUAL_SENSOR_ERROR 			-2
#define DUAL_SENSOR_INCONSISTENT    -3



int32_t DualChannelInit();

int32_t DualChannelUpdate();

int32_t DualChannelSetVoltage();

int32_t DualChannelVoltageAverage();

int32_t ppmThresholdChecking();

int32_t gasSensorHandler();




#endif /* SRC_SERVICE_DUALCHANNELGASSENSOR_H_ */
