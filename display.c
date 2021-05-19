//*****************************************************************************
//
// display.c
//
// Functionality for displaying helicopter data using
// OLED display. Displays current height and yaw, as well
// as main and tail duty cycles.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified: 19/05/2021
//
// Sourced code acknowledged in function descriptions

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
displayData(int16_t height_percent, int32_t display_deg, duty_cycle_s heli_duty)
{
    char string[17];  // 16 characters across the display

    // Print each line of OLED display data
    usnprintf (string, sizeof(string), "Height   %5d%%", height_percent);
    OLEDStringDraw (string, 0, 0);
    usnprintf (string, sizeof(string), "Yaw (deg) %5d", display_deg);
    OLEDStringDraw (string, 0, 1);
    usnprintf (string, sizeof(string), "Main Duty %5d%%", heli_duty.main);
    OLEDStringDraw (string, 0, 2);
    usnprintf (string, sizeof(string), "Tail Duty %5d%%", heli_duty.tail);
    OLEDStringDraw (string, 0, 3);
}

