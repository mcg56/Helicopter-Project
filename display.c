//*****************************************************************************
//
// display.c - Display functionality
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Date 19/04/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stdlib.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "display.h"

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
displayMeanVal(int16_t height_percent, int32_t display_deg, int16_t count, int16_t Acount)
{
    char string[17];  // 16 characters across the display

    usnprintf (string, sizeof(string), "Height   %5d%%", height_percent);
    OLEDStringDraw (string, 0, 0);
    usnprintf (string, sizeof(string), "Yaw (deg) %5d", display_deg);
    OLEDStringDraw (string, 0, 1);
    usnprintf (string, sizeof(string), "Yaw (deg) %5d", count);
    OLEDStringDraw (string, 0, 2);
    usnprintf (string, sizeof(string), "Yaw (deg) %5d", Acount);
        OLEDStringDraw (string, 0, 3);
}

