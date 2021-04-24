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
#define BUF_SIZE            10   // Buffer size for sample averaging
#define SAMPLE_RATE_HZ      2000 // Sample rate
#define SYSTICK_RATE_HZ     100  // Systick configuration
#define ADC_BITS            4095 // 12 bit ADC


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
initDisplay (void);

//*****************************************************************************
// Function to control display
//*****************************************************************************
void
displayMeanVal(uint16_t mean_val, uint16_t landed_height, displayType display_state);



#endif /* DISPLAY_H_ */
