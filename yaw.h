#ifndef YAW_H_
#define YAW_H_

// *******************************************************
// yaw.h
//
// Support for yaw functionality of the helicopter
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Last modified:  24.4.2021
//
// *******************************************************

#include <stdint.h>
#include <stdio.h>

int16_t count;

void calculateYaw(bool a_next, bool b_next);


//*************************************************************
// Intialise GPIO Pins
//*************************************************************
void
initGPIOPins (void);

//*************************************************************
// Intialise Yaw module
//*************************************************************
void
initYaw (void);

//*****************************************************************************
// Function to update helicopter yaw in degrees
//*****************************************************************************
void
calculateYaw(bool a_next, bool b_next);

//*****************************************************************************
// Update yaw helicopter control
//*****************************************************************************
uint32_t
updateYaw(int16_t yaw_degree_in, int16_t target_yaw_in);

//*****************************************************************************
// Pass deg to main function
//*****************************************************************************
int
getYaw(void);

void
findReference(void);

//*****************************************************************************
// Pass yaw data to response control module
//*****************************************************************************
int16_t
getYawData(void);

//*****************************************************************************
// Pass yaw data to response control module
// TO BE REMOVED
//*****************************************************************************
int16_t
getYawDataTarget(void);


#endif /* YAW_H_ */
