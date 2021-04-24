#ifndef YAW_H_
#define YAW_H_

// *******************************************************
// yaw.h
//
// Support for a set of FOUR specific buttons on the Tiva/Orbit.
// ENCE361 sample code.
// The buttons are:  UP and DOWN (on the Orbit daughterboard) plus
// LEFT and RIGHT on the Tiva.
//
// P.J. Bones UCECE
// Last modified:  7.2.2018
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>

void updateYaw(bool a_next, bool b_next);

//*************************************************************
// GPIO Pin Interrupt
//*************************************************************
void
GPIOPinIntHandler (void)

//*************************************************************
// Intialise GPIO Pins
//*************************************************************
void
initGPIOPins (void)

//*****************************************************************************
// Function to update helicopter yaw in degrees
//*****************************************************************************
void
updateYaw(bool a_next, bool b_next)




#endif /* YAW_H_ */
