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


// Debounce algorithm: A state machine is associated with each button.
// A state change occurs only after NUM_BUT_POLLS consecutive polls have
// read the pin in the opposite condition, before the state changes and
// a flag is set.  Set NUM_BUT_POLLS according to the polling rate.

// *******************************************************
// initButtons: Initialise the variables associated with the set of buttons
// defined by the constants above.
void
initSwitches (void);

// *******************************************************
// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
void
updateSwitches (void);

// *******************************************************
// checkButton: Function returns the new button state if the button state
// (PUSHED or RELEASED) has changed since the last call, otherwise returns
// NO_CHANGE.  The argument butName should be one of constants in the
// enumeration butStates, excluding 'NUM_BUTS'. Safe under interrupt.
uint8_t
checkSwitch (uint8_t switchName);

// *******************************************************
// Return current switch state
// *******************************************************
flight_mode
switchValue(void);

#endif /*SWITCH_H_*/
