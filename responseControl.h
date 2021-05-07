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
#define PROPORTIONAL_GAIN_MAIN      4 // Proportional control gain for main rotor
#define PROPORTIONAL_GAIN_TAIL      1  // Proportional control gain for tail rotor
#define INTEGRAL_GAIN_MAIN          0  // Integral control gain for main rotor
#define INTEGRAL_GAIN_TAIL          0  // Integral control gain for tail rotor
#define MAX_DUTY_MAIN                    97 // Maximum helicopter duty cycle
#define MIN_DUTY_MAIN                    2  // Minimum helicopter duty cycle
#define OFFSET_DUTY                      30 // Helicopter hover duty
#define MAX_DUTY_TAIL                    97  // Minimum helicopter duty cycle
#define MIN_DUTY_TAIL                    2  // Minimum helicopter duty cycle

int16_t countbla;

//*****************************************************************************
// Caculate helicopter main rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseMain(int16_t current_height, int16_t target_height);

//*****************************************************************************
// Caculate helicopter tail rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseTail(int16_t current_yaw, int16_t target_yaw);

//*****************************************************************************
// Intialise timer for PI control update
//*****************************************************************************
void
initResponseTimer (void);

uint32_t
getTailDuty(void);

uint32_t
getMainDuty(void);

#endif /* RESPONSECONTROL_H_ */
