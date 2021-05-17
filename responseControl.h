#ifndef RESPONSECONTROL_H_
#define RESPONSECONTROL_H_

// *******************************************************
// responseControl.h
//
// Support for response control of the helicopter rotors
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Last modified:  24.4.2021
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "altitude.h"
#include "yaw.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define PROPORTIONAL_GAIN_MAIN      0.3 // Proportional control gain for main rotor
#define PROPORTIONAL_GAIN_TAIL      0.1  // Proportional control gain for tail rotor
#define INTEGRAL_GAIN_MAIN          0.00028  // Integral control gain for main rotor
#define INTEGRAL_GAIN_TAIL          0.00008  // Integral control gain for tail rotor
#define MAX_DUTY_MAIN                    98 // Maximum helicopter duty cycle
#define MIN_DUTY_MAIN                    2  // Minimum helicopter duty cycle
#define COUPLING_RATIO                   22 // Helicopter hover duty
#define MAX_DUTY_TAIL                    97  // Minimum helicopter duty cycle
#define MIN_DUTY_TAIL                    2  // Minimum helicopter duty cycle
#define TIMER_RATE                       1000 // Determines rate of timer interrupt

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
// Pass PWM main and tail duty out of module
//*****************************************************************************
duty_cycle_s
getHeliDuty(void);

#endif /* RESPONSECONTROL_H_ */
