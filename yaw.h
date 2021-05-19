#ifndef YAW_H_
#define YAW_H_

// *******************************************************
// yaw.h
//
// Helicopter yaw functionality. Reads and updates yaw via interrupts to GPIO
// pins reading from a quadrature encoder disk. Yaw range is 180 to -179.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified: 19/5/2021
//
// *******************************************************

#include <stdint.h>
#include <stdio.h>

//*************************************************************
// Type definitions
//*************************************************************
typedef struct {
    int16_t current;
    int16_t target;
} yaw_data_s;

//*************************************************************
// Intialise Yaw module
//*************************************************************
void
initYaw (void);

//*****************************************************************************
// Sweep helicopter to find reference yaw
//*****************************************************************************
bool
findReference(void);

//*****************************************************************************
// Pass current yaw out of module
//*****************************************************************************
int16_t
getYawCurrent(void);

//*****************************************************************************
// Pass reference found out of module
//*****************************************************************************
bool
refFound(void);

#endif /* YAW_H_ */
