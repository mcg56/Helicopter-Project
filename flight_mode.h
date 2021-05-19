#ifndef FLIGHT_MODE_H_
#define FLIGHT_MODE_H_

// *******************************************************
//
// flight_mode.h
//
// Reads and updates the value of the switch and updates the
// helicopter state accordingly.
//
//
// Note that pin PF0 (the pin for the RIGHT pushbutton - SW2 on
//  the Tiva board) needs special treatment - See PhilsNotesOnTiva.rtf.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified:  20/5/2021
//
// Sourced code acknowledged in function descriptions
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>

// SWITCH ONE
#define SWITCH_ONE_PERIPH  SYSCTL_PERIPH_GPIOA
#define SWITCH_ONE_PORT_BASE  GPIO_PORTA_BASE
#define SWITCH_ONE_PIN  GPIO_PIN_7
#define SWITCH_ONE_NORMAL  true
#define NUM_SWITCH_POLLS 3

//*****************************************************************************
// Definition Types
//*****************************************************************************
// Helicopter FSM states
typedef enum {
    landed,
    initialising,
    flying,
    landing,
} flight_mode;

//*****************************************************************************
// Initialise switch
// Sourced from: P.J. Bones UCECE
//*****************************************************************************
void
initSwitches (void);

// *******************************************************
// Update and return current helicopter state
// *******************************************************
flight_mode
updateState(flight_mode main_state);

// *******************************************************
// Pass helicopter state out of module
// *******************************************************
flight_mode
getState(void);

#endif /*FLIGHT_MODE_H_*/
