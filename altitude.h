#ifndef ALTITUDE_H_
#define ALTITUDE_H_

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
#include "circBufT.h"

extern circBuf_t g_inBuffer;

//*****************************************************************************
// Constants
//*****************************************************************************
#define BUF_SIZE            10   // Buffer size for sample averaging
#define SAMPLE_RATE_HZ      2000 // Sample rate
#define SYSTICK_RATE_HZ     100  // Systick configuration
#define ADC_BITS            4095 // 12 bit ADC

//*****************************************************************************
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
// Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
void
ADCIntHandler(void);

//*****************************************************************************
// Initialise ADC functions
// Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
void
initADC (void);

//*****************************************************************************
// Function to convert helicopter height to percentage
//*****************************************************************************
int
calculate_percent_height(uint16_t current_height, uint16_t landed_height);

//*****************************************************************************
// Function to record helicopter landed height.
//*****************************************************************************
int
calibrate_height();

#endif /* ALTITUDE_H_ */
