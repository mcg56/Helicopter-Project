#ifndef ALTITUDE_H_
#define ALTITUDE_H_

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
#define BUF_SIZE            10   // Buffer size for sample averaging
#define ADC_BITS            4095 // 12 bit ADC

//*****************************************************************************
// Initialise altitude module
//*****************************************************************************
extern void
initAltitude(void);

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
extern void
initADC (void);

//*****************************************************************************
// Function to convert helicopter height to percentage
//*****************************************************************************
extern int
calculate_percent_height(uint16_t current_height, uint16_t landed_height);

//*****************************************************************************
// Find and return current helicopter landed height.
//*****************************************************************************
extern int
getHeight(void);

//*****************************************************************************
// Update helicopter altitute control
//*****************************************************************************
extern void
updateAltitude(uint32_t pwm_main_duty);

#endif /* ALTITUDE_H_ */
