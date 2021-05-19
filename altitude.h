#ifndef ALTITUDE_H_
#define ALTITUDE_H_

// *******************************************************
// altitude.h
//
// Support for altitude functionality of the helicopter
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified:  19/5/2021
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
// Return helicopter height as a percentage of total range
//*****************************************************************************
int
calculate_percent_height(uint16_t current_height_in, uint16_t landed_height_in);

//*****************************************************************************
// Determine current helicopter height in ADC value
//*****************************************************************************
int
getHeight(void);

#endif /* ALTITUDE_H_ */
