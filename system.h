#ifndef SYSTEM_H_
#define SYSTEM_H_

// *******************************************************
// system.h
//
// Support for system functionality of the helicopter
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Last modified:  24.4.2021
//
// *******************************************************

//*****************************************************************************
// Constants
//*****************************************************************************
#define SAMPLE_RATE_HZ      2000 // Sample rate

//*****************************************************************************
// Initialisation function for the clock
// Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
void
initClock (void);

//*****************************************************************************
// Initialisation function for soft reset pin
//*****************************************************************************
void
initSoftReset (void);

//*************************************************************
// Pass slowTick to main for UART transmission
//*************************************************************
uint8_t
getSlowTick(void);

#endif /* SYSTEM_H_ */
