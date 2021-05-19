#ifndef DISPLAY_H_
#define DISPLAY_H_

// *******************************************************
// display.h
//
// Functionality for displaying helicopter data using
// OLED display. Displays current height and yaw, as well
// as main and tail duty cycles.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified:  19/5/2021
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "responseControl.h"

//*****************************************************************************
// Initialise Display Function
// Code Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
void
initDisplay (void);

//*****************************************************************************
// Update display
//*****************************************************************************
void
displayData(int16_t height_percent, int32_t display_deg, duty_cycle_s heli_duty);

#endif /* DISPLAY_H_ */
