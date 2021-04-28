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


void updateYaw(bool a_next, bool b_next);

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
updateYaw(bool a_next, bool b_next);

//*****************************************************************************
// Pass deg to main function
//*****************************************************************************
int
getYaw(void);


#endif /* YAW_H_ */
