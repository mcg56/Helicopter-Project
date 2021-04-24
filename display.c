//*****************************************************************************
//
// display.c - display functionality
//
// Authors: Tom Peterson, Matt Comber, Mark Gardyne
// Date 19/04/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)

#include <stdint.h> /
#include <stdio.h>  /
#include <stdbool.h>  /
#include "stdlib.h"  /
#include "inc/hw_memmap.h"  /
#include "inc/hw_types.h"  /
#include "inc/hw_ints.h"    /
#include "driverlib/adc.h"  /
#include "driverlib/gpio.h" /
#include "driverlib/sysctl.h"   /
#include "driverlib/systick.h"  /
#include "driverlib/interrupt.h"    /
#include "driverlib/debug.h"    /
#include "driverlib/pin_map.h"  /
#include "utils/ustdlib.h"  /
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"   /


//*****************************************************************************
// Constants
//*****************************************************************************
#define BUF_SIZE            10   // Buffer size for sample averaging
#define SAMPLE_RATE_HZ      2000 // Sample rate
#define SYSTICK_RATE_HZ     100  // Systick configuration
#define ADC_BITS            4095 // 12 bit ADC

//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;        // Counter for the interrupts
static int32_t yaw;                 // Helicopter heading from quadrature code disc
static int32_t deg;                 // Helicopter heading in degrees
static bool a_cur;                  // Current A-phase pin value
static bool b_cur;                  // Current B-phase pin value

typedef enum {
    percent_height,
    ADC_height,
    off
} displayType;

//*****************************************************************************
// Initialise Display Function
// Code Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
void
initDisplay (void)
{
    // intialise the Orbit OLED display
    OLEDInitialise ();
}

//*****************************************************************************
// Function to control display
//*****************************************************************************
void
displayMeanVal(uint16_t mean_val, uint16_t landed_height, displayType display_state)
{
    char string[17];  // 16 characters across the display
    int16_t height_percent;

    switch (display_state)
    {
    case percent_height:
        height_percent = calculate_percent_height(mean_val, landed_height);
        usnprintf (string, sizeof(string), "Height   %5d%%", height_percent);
        OLEDStringDraw (string, 0, 0);
        usnprintf (string, sizeof(string), "Yaw (deg) %5d", deg);
        OLEDStringDraw (string, 0, 1);
        break;
    case ADC_height:
        usnprintf (string, sizeof(string), "Mean ADC = %4d", mean_val);
        OLEDStringDraw (string, 0, 0);
        break;
    case off:
        OLEDStringDraw ("                ", 0, 0);
        OLEDStringDraw ("                ", 0, 1);
    }
}
