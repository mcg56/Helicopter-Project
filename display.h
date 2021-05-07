#ifndef DISPLAY_H_
#define DISPLAY_H_

// *******************************************************
// altitude.h
//
// Support for altitude functionality of the helicopter
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Last modified:  24.4.2021
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
// Constants
//*****************************************************************************
#define SAMPLE_RATE_HZ      2000 // Sample rate
#define SYSTICK_RATE_HZ     100  // Systick configuration
#define ADC_BITS            4095 // 12 bit ADC

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
displayMeanVal(int16_t height_percent, int32_t display_deg, int var_check);

#endif /* DISPLAY_H_ */
