#ifndef RESPONSECONTROL_H_
#define RESPONSECONTROL_H_

// *******************************************************
// responseControl.h
//
// Motion control for helicopter. Takes current helicopter state, position and
// target position from the main and drives the rotors appropriately.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified: 19/5/2021
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "altitude.h"
#include "yaw.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define MAX_DUTY_MAIN               98   // Maximum helicopter duty cycle
#define MIN_DUTY_MAIN               28    // Minimum helicopter duty cycle
#define COUPLING_OFFSET             25   // Helicopter hover duty
#define MAX_DUTY_TAIL               97   // Minimum helicopter duty cycle
#define MIN_DUTY_TAIL               2    // Minimum helicopter duty cycle
#define TIMER_RATE                  2000 // Determines rate of timer interrupt

//*****************************************************************************
// Type definitions
//*****************************************************************************
typedef struct {
    uint32_t main;
    uint32_t tail;
} duty_cycle_s;

//*****************************************************************************
// Intialise timer for PI control update
//*****************************************************************************
void
initResponseTimer (void);

//*****************************************************************************
// Update PWM based on helicopter state
//*****************************************************************************
void
updateResponseControl (height_data_s height_data, yaw_data_s yaw_data);

//*****************************************************************************
// Pass PWM main and tail duties out of module
//*****************************************************************************
duty_cycle_s
getHeliDuty(void);

#endif /* RESPONSECONTROL_H_ */
