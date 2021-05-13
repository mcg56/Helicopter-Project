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
#include "responseControl.h"

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
displayMeanVal(int16_t height_percent, int32_t display_deg, duty_cycle_s heli_duty)
{
    char string[17];  // 16 characters across the display

    usnprintf (string, sizeof(string), "Height   %5d%%", height_percent);
    OLEDStringDraw (string, 0, 0);
    usnprintf (string, sizeof(string), "Yaw (deg) %5d", display_deg);
    OLEDStringDraw (string, 0, 1);
    usnprintf (string, sizeof(string), "Main Duty %5d%%", heli_duty.main);
    OLEDStringDraw (string, 0, 2);
    usnprintf (string, sizeof(string), "Tail Duty %5d%%", heli_duty.tail);
    OLEDStringDraw (string, 0, 3);
}

