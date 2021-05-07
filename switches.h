#ifndef SWITCHES_H_
#define SWITCHES_H_

// *******************************************************
//
// switches.h
//
// Support for the up button Tiva/Orbit.
// ENCE361 sample code.
//
//
// Note that pin PF0 (the pin for the RIGHT pushbutton - SW2 on
//  the Tiva board) needs special treatment - See PhilsNotesOnTiva.rtf.
//
// TRP
// Last modified:  26.04.2020
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>

// SWITCH ONE
#define SWITCH_ONE_PERIPH  SYSCTL_PERIPH_GPIOA
#define SWITCH_ONE_PORT_BASE  GPIO_PORTA_BASE
#define SWITCH_ONE_PIN  GPIO_PIN_7
#define SWITCH_ONE_NORMAL  false
#define NUM_SWITCH_POLLS 3

//*****************************************************************************
// Definition Types
//*****************************************************************************
typedef enum {
    landed,
    take_off,
} flight_mode;


//*****************************************************************************
// Initialise switch
//*****************************************************************************
void
initSwitches (void);

// *******************************************************
// Return current switch state
// *******************************************************
flight_mode
switchValue(void);

#endif /*SWITCH_H_*/
