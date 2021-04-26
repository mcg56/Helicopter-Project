// *******************************************************
//
// switches.c
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
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "inc/tm4c123gh6pm.h"  // Board specific defines (for PF0)
#include "switches.h"


// *******************************************************
// Globals to module
// *******************************************************
static bool switch_state[NUM_SWITCHES];    // Corresponds to the electrical state
static uint8_t switch_count[NUM_SWITCHES];
static bool switch_flag[NUM_SWITCHES];
static bool switch_normal[NUM_SWITCHES];   // Corresponds to the electrical state

// *******************************************************
// initButtons: Initialise the variables associated with the set of buttons
// defined by the constants in the buttons2.h header file.
void
initSwitches (void)
{
    int i;

    // UP button (active HIGH)
    SysCtlPeripheralEnable (SWITCH_ONE_PERIPH);
    GPIOPinTypeGPIOInput (SWITCH_ONE_PORT_BASE, SWITCH_ONE_PIN);
    GPIOPadConfigSet (SWITCH_ONE_PORT_BASE, SWITCH_ONE_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    switch_normal[ONE] = SWITCH_ONE_NORMAL; // Low at begining when switch is down


    for (i = 0; i < NUM_SWITCHES; i++)
    {
        switch_state[i] = switch_normal[i];
        switch_count[i] = 0;
        switch_flag[i] = false;
    }
}

// *******************************************************
// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
// Debounce algorithm: A state machine is associated with each button.
// A state change occurs only after NUM_BUT_POLLS consecutive polls have
// read the pin in the opposite condition, before the state changes and
// a flag is set.  Set NUM_BUT_POLLS according to the polling rate.
void
updateSwitches (void)
{
    bool switch_value[NUM_SWITCHES];
    int i;

    // Read the pins; true means HIGH, false means LOW
    switch_value[ONE] = (GPIOPinRead (SWITCH_ONE_PORT_BASE, SWITCH_ONE_PIN) == SWITCH_ONE_PIN);

    // Iterate through the buttons, updating button variables as required
    for (i = 0; i < NUM_SWITCHES; i++)
    {
        if (switch_value[i] != switch_state[i])
        {
            switch_count[i]++;
            if (switch_count[i] >= NUM_SWITCH_POLLS)
            {
                switch_state[i] = switch_value[i];
                switch_flag[i] = true;    // Reset by call to checkButton()
                switch_count[i] = 0;
            }
        }
        else
            switch_count[i] = 0;
    }
}

// *******************************************************
// checkButton: Function returns the new button logical state if the button
// logical state (PUSHED or RELEASED) has changed since the last call,
// otherwise returns NO_CHANGE.
uint8_t
checkSwitch (uint8_t switchName)
{
    if (switch_flag[switchName])
    {
        switch_flag[switchName] = false;
        if (switch_state[switchName] == switch_normal[butName])
            return HIGH;
        else
            return LOW;
    }
    return NO_CHANGE;
}

