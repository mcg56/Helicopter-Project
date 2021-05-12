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
// Structure definitions
//*****************************************************************************
typedef struct {
    int16_t current;
    int16_t target;
} height_data_s;

//*****************************************************************************
// Initialise altitude module
//*****************************************************************************
void
initAltitude(void);

//*****************************************************************************
// Convert helicopter height to percentage
//*****************************************************************************
int
calculate_percent_height(uint16_t current_height_in, uint16_t landed_height_in);

//*****************************************************************************
// Find current helicopter height.
//*****************************************************************************
int
getHeight(void);

//*****************************************************************************
// Update helicopter altitute control
//*****************************************************************************
uint32_t
updateAltitude(height_data_s height_data_in);

//*****************************************************************************
// Passes height data to reponse module
//*****************************************************************************
height_data_s
getAltitudeData(void);

#endif /* ALTITUDE_H_ */
