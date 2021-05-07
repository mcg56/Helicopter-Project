//*****************************************************************************
//
// display.c - Display functionality
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Date 19/04/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)

#include <stdint.h>
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "display.h"

//#include "driverlib/pin_map.h"
//#include "driverlib/debug.h"
//#include "driverlib/interrupt.h"
//#include "driverlib/systick.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/gpio.h"
//#include "driverlib/adc.h"
//#include "inc/hw_ints.h"
//#include <stdio.h>
//#include <stdbool.h>
//#include "stdlib.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_types.h"

//*****************************************************************************
// Initialise Display Function
// Code Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
void
initDisplay (void)
{
    // Intialise the Orbit OLED display
    OLEDInitialise ();
}

//*****************************************************************************
// Update display
//*****************************************************************************
void
displayMeanVal(int16_t height_percent, int32_t display_deg)
{
    char string[17];  // 16 characters across the display

    usnprintf (string, sizeof(string), "Height   %5d%%", height_percent);
    OLEDStringDraw (string, 0, 0);
    usnprintf (string, sizeof(string), "Yaw (deg) %5d", display_deg);
    OLEDStringDraw (string, 0, 1);
}

