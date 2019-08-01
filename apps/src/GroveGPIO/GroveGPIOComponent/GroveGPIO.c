/** GPIO mapping **/
/**
 * GPIO42 -- IOT0_GPIO1 (D2)
 * GPIO13 -- IOT0_GPIO2 (D3)
 * GPIO7  -- IOT0_GPIO3 (D4)
 * GPIO8  -- IOT0_GPIO4 (D5)
 * ADC0   -- A0
 * I2C_HUB1 -- I2C
 * UART1  -- UART
 * 
 **/
#include "legato.h"
#include "interfaces.h"

#include <string.h>

typedef enum IOT0_GPIO_PIN {
	D2 = 0,
	D3,
	D4,
	D5
} IOT0_GPIO_PIN_t;

#if 0
static void GPIO_TestOutput(void)
{
	led_matrix_displayString("Test GPIO Output", 700 * strlen("Test GPIO Output"), true, LED_MATRIX_GREEN);
	// sleep(20);
	int i = 0;

	if (D2_IsOutput()){
		// D2_Activate();
	} else {
		D2_SetPushPullOutput(D2_ACTIVE_HIGH, true);
	}
	if (D3_IsOutput()){
		// D3_Activate();
	} else {
		D3_SetPushPullOutput(D3_ACTIVE_HIGH, true);
	}
	if (D2_IsOutput()){
		// D4_Activate();
	} else {
		D4_SetPushPullOutput(D4_ACTIVE_HIGH, true);
	}
	if (D5_IsOutput()){
		// D5_Activate();
	} else {
		D5_SetPushPullOutput(D5_ACTIVE_HIGH, true);
	}

	for ( i = 0; i < 100; i++) {
		D2_Activate();
		D3_Activate();
		D4_Activate();
		D5_Activate();
		sleep(1);
		D2_Deactivate();
		D3_Deactivate();
		D4_Deactivate();
		D5_Deactivate();
		sleep(1);
	}
}

static void GPIO_TestInput()
{
	led_matrix_displayString("Test GPIO Input", 700 * strlen("Test GPIO Input"), true, LED_MATRIX_GREEN);
	sleep(10);
}
#endif 

enum GPIO_MODE {
	INPUT,
	OUTPUT
};
enum GPIO_ACTION {
	LOW = 0,
	HIGH,
	READ
};
static int port = D2;
static int mode = INPUT;
static int action = READ;

static void PrintHelp()
{
	puts(   "NAME\n"
		"        GroveGPIO - output/input GPIO on IOT grove card.\n"
		"\n"
		"SYNOPSIS\n"
		"        GroveGPIO OPTION MODE ACTION\n"
		"        GroveGPIO -h\n"
		"        GroveGPIO --help\n"
		"\n"
		"MODE\n"
		"       input\n"
		"               GPIO is set as input.\n"
		"       output\n"
		"               GPIO is set as output.\n"
		"ACTION\n"
		"       read\n"
		"               Read input value.\n"
		"       high\n"
		"               Output HIGH.\n"
		"       low\n"
		"               Output LOW.\n"
		"OPTIONS\n"
		"       -p--port\n"
		"       --port\n"
		"               GPIO port number.\n"
		"\n"
		"       help\n"
		"               Print a help message and exit. Ignore all other arguments.\n"
		"\n"
		);

	exit(EXIT_SUCCESS);
}

static void GPIO_mode(const char * arg)
{
	if (strcmp("input", arg) == 0)
	{
		mode = INPUT;
	}
	else if (strcmp("output", arg) == 0)
	{
		mode = OUTPUT;
	}
	else if (strcmp("help", arg) == 0)
	{
		PrintHelp();
	}
	else
	{
		fprintf(stderr, "Unrecognized command: '%s'\n", arg);
		exit(EXIT_FAILURE);
	}
}
static void GPIO_action(const char * arg)
{
	if (strcmp("read", arg) == 0)
	{
		action = READ;
	}
	else if (strcmp("high", arg) == 0)
	{
		action = HIGH;
	}
	else if (strcmp("low", arg) == 0)
	{
		action = LOW;
	}
	else
	{
		fprintf(stderr, "Unrecognized command: '%s'\n", arg);
		exit(EXIT_FAILURE);
	}
}

COMPONENT_INIT
{
	LE_INFO("IOT0 Grove Card GPIO");

	le_arg_SetIntVar(&port, "p", "port");
	le_arg_SetFlagCallback(PrintHelp, "h", "help");
	le_arg_AddPositionalCallback(GPIO_mode);
	le_arg_AddPositionalCallback(GPIO_action);

	LE_INFO("Scanning.");
	// Perform command-line argument processing.
	le_arg_Scan();
	LE_INFO("Done scanning.");

	switch (port) {
	case D2:
		if (mode == INPUT) {
			// only read action is supported
			if (action == READ) {
				// TODO: will be implement if needed
			} else {
				exit(EXIT_FAILURE);
			}
		} else if (mode == OUTPUT) {
			D2_SetPushPullOutput(D2_ACTIVE_HIGH, action);
		} else {
			exit(EXIT_FAILURE);
		}
		break;
	case D3:
		if (mode == INPUT) {
			// only read action is supported
			if (action == READ) {
				// TODO: will be implement if needed
			} else {
				exit(EXIT_FAILURE);
			}
		} else if (mode == OUTPUT) {
			D3_SetPushPullOutput(D3_ACTIVE_HIGH, action);
		} else {
			exit(EXIT_FAILURE);
		}
		break;
	case D4:
		if (mode == INPUT) {
			// only read action is supported
			if (action == READ) {
				// TODO: will be implement if needed
			} else {
				exit(EXIT_FAILURE);
			}
		} else if (mode == OUTPUT) {
			D4_SetPushPullOutput(D4_ACTIVE_HIGH, action);
		} else {
			exit(EXIT_FAILURE);
		}
		break;
	case D5:
		if (mode == INPUT) {
			// only read action is supported
			if (action == READ) {
				// TODO: will be implement if needed
			} else {
				exit(EXIT_FAILURE);
			}
		} else if (mode == OUTPUT) {
			D5_SetPushPullOutput(D5_ACTIVE_HIGH, action);
		} else {
			exit(EXIT_FAILURE);
		}
		break;
	default:
		fprintf(stderr, "Unsupported output: '%d'\n", port);
		exit(EXIT_FAILURE);
		break;
	}
	exit(EXIT_SUCCESS);
}