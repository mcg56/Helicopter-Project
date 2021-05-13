// *******************************************************
//
// switches.c
//
// Support for the up button Tiva/Orbit.
// ENCE361 sample code.
//
// Note that pin PF0 (the pin for the RIGHT pushbutton - SW2 on
//  the Tiva board) needs special treatment - See PhilsNotesOnTiva.rtf.
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Last modified:  26.04.2020
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "switches.h"


//*****************************************************************************
// Enumerated types
//*****************************************************************************
enum switchNames {ONE = 0, NUM_SWITCHES};
enum switchStates {LOW = 0, HIGH, NO_CHANGE};

// *******************************************************
// Globals Variables
// *******************************************************
static bool switch_normal[NUM_SWITCHES];   // Corresponds to the electrical state
static bool switch_state[NUM_SWITCHES];    // Corresponds to the electrical state
static bool switch_flag[NUM_SWITCHES];
static uint8_t switch_count[NUM_SWITCHES];
static uint8_t switchState;
static flight_mode current_state;

//*****************************************************************************
// Initialise switch
//*****************************************************************************
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

    // Sets initial switch value to false
    for (i = 0; i < NUM_SWITCHES; i++)
    {
        switch_state[i] = switch_normal[i];
        switch_count[i] = 0;
        switch_flag[i] = false;
    }
}

//*****************************************************************************
// Update switch value with debouncing
//*****************************************************************************
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

//*****************************************************************************
// Checks if switch state has changed
//*****************************************************************************
uint8_t
checkSwitch (uint8_t switchName)
{
    if (switch_flag[switchName])
    {
        switch_flag[switchName] = false;
        if (switch_state[switchName] == switch_normal[switchName])
            return HIGH;
        else
            return LOW;
    }
    return NO_CHANGE;
}

// *******************************************************
// Update and return current helicopter state
// *******************************************************
flight_mode
updateState(flight_mode main_state)
{
    // Update state from main
    current_state = main_state;

    // Update switches
    updateSwitches();
    switchState = checkSwitch (ONE);

    switch (switchState)
    {
    case NO_CHANGE:
        break;
    case HIGH:
        // Prevent state change during landing process
        if (current_state == landed) {
            current_state = flying;
        }
        break;
    case LOW:
        if (current_state == flying) {
            current_state = landing;
        }
        break;
    }

    return current_state;
}

// *******************************************************
// Return current helicopter state
// *******************************************************
flight_mode
getState(void)
{
    return current_state;
}
