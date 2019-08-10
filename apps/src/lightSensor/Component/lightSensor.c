//--------------------------------------------------------------------------------------------------
/**
 * Implementation of the mangOH Red Gas sensor interface.
 *
 * Provides the gas API services and plugs into the Legato Data Hub.
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//--------------------------------------------------------------------------------------------------
#include "legato.h"
#include "interfaces.h"

const char lightSensorAdc[] = "EXT_ADC3";

le_result_t lightSensor_Read(int *ret_val)
{
	le_result_t r = le_adc_ReadValue(lightSensorAdc, ret_val);

	if (r != LE_OK)
	{
		return r;
	}

	return LE_OK;
}

COMPONENT_INIT
{
	int valueMv;
	char valueMv_str[16];
	if (lightSensor_Read(&valueMv) != LE_OK) {
		puts("-1\n");
		exit(EXIT_FAILURE);
	}
	memset(valueMv_str, 0, 16);
	sprintf(valueMv_str, "%i\n", valueMv);
	puts(valueMv_str);
	exit(EXIT_SUCCESS);
}
