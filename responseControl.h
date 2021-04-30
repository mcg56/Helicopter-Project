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
#include "stdlib.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define PROPORTIONAL_GAIN_MAIN      1  // Proportional control gain for main rotor
#define PROPORTIONAL_GAIN_TAIL      0.75  // Proportional control gain for tail rotor
#define MAX_DUTY                    90 // Maximum helicopter duty cycle
#define MIN_DUTY                    3  // Minimum helicopter duty cycle

//*****************************************************************************
// Caculate helicopter main rotor response using PI control
//*****************************************************************************
int
dutyResponseMain(int16_t current_height, int16_t target_height);

//*****************************************************************************
// Caculate helicopter tail rotor response using PI control
//*****************************************************************************
int
dutyResponseTail(int16_t current_yaw, int16_t target_yaw);

#endif /* RESPONSECONTROL_H_ */
