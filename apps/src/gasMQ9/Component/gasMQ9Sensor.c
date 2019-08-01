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

const char gasMQ9SensorAdc[] = "EXT_ADC0";

//--------------------------------------------------------------------------------------------------
/**
 * Read Gas measurement
 *
 * @return LE_OK if successful.
 */
//--------------------------------------------------------------------------------------------------

le_result_t gasMQ9_Read(double *gas_value)
{
	int32_t valueMv;

	le_result_t r = le_adc_ReadValue(gasMQ9SensorAdc, &valueMv);

	if (r != LE_OK)
	{
		return r;
	}

	*gas_value = valueMv / 1000.0;

	return LE_OK;
}

COMPONENT_INIT
{
	double valueMv;
	char valueMv_str[16];
	if (gasMQ9_Read(&valueMv) != LE_OK) {
		// fprintf(stderr, "Did not find fingerprint sensor :(\n");
		exit(EXIT_FAILURE);
	}
	memset(valueMv_str, 0, 16);
	sprintf(valueMv_str, "%f\n", valueMv);
	puts(valueMv_str);
	exit(EXIT_SUCCESS);
}
