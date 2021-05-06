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
void
initAltitude(void);

//*****************************************************************************
// Function to convert helicopter height to percentage
//*****************************************************************************
int
calculate_percent_height(uint16_t current_height_in, uint16_t landed_height_in);

//*****************************************************************************
// Find and return current helicopter landed height.
//*****************************************************************************
int
getHeight(void);

//*****************************************************************************
// Update helicopter altitute control
//*****************************************************************************
uint32_t
updateAltitude(int16_t height_percent, int16_t target_height_percent);

//*****************************************************************************
// Passes current height and target height to reponse module
//*****************************************************************************
int16_t
getAltitudeData(void);

//*****************************************************************************
// Passes current height and target height to reponse module
// TO BE REMOVED
//*****************************************************************************
int16_t
getAltitudeDataTarget(void);

#endif /* ALTITUDE_H_ */
