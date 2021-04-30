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


void calculateYaw(bool a_next, bool b_next);

//*************************************************************
// GPIO Pin Interrupt
//*************************************************************
void
GPIOPinIntHandler (void);

//*************************************************************
// Intialise GPIO Pins
//*************************************************************
void
initGPIOPins (void);

//*****************************************************************************
// Function to update helicopter yaw in degrees
//*****************************************************************************
void
calculateYaw(bool a_next, bool b_next);

//*****************************************************************************
// Update yaw helicopter control
//*****************************************************************************
uint32_t
updateYaw(int16_t yaw_degree, int16_t target_yaw);

//*****************************************************************************
// Pass deg to main function
//*****************************************************************************
int
getYaw(void);


#endif /* YAW_H_ */
