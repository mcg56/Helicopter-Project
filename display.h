#ifndef DISPLAY_H_
#define DISPLAY_H_

// *******************************************************
// altitude.h
//
// Support for altitude functionality of the helicopter
//
// T.R. Peterson M.G. Gardyne
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
// Display cases
//*****************************************************************************
typedef enum {
    percent_height,
    ADC_height,
    off
} displayType;

//*****************************************************************************
// Initialise Display Function
// Code Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
extern void
initDisplay (void);

//*****************************************************************************
// Function to control display
//*****************************************************************************
extern void
displayMeanVal(uint16_t mean_val, int16_t height_percent, displayType display_state, int32_t display_deg);

#endif /* DISPLAY_H_ */
