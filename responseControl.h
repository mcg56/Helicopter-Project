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


//*****************************************************************************
// Constants
//*****************************************************************************
#define PROPORTIONAL_GAIN_MAIN      0.6 // Proportional control gain for main rotor
#define PROPORTIONAL_GAIN_TAIL      0.7  // Proportional control gain for tail rotor
#define INTEGRAL_GAIN_MAIN          0.0005  // Integral control gain for main rotor
#define INTEGRAL_GAIN_TAIL          0.0003  // Integral control gain for tail rotor
#define MAX_DUTY_MAIN                    98 // Maximum helicopter duty cycle
#define MIN_DUTY_MAIN                    20  // Minimum helicopter duty cycle
#define OFFSET_DUTY_MAIN                 30 // Helicopter hover duty
#define OFFSET_DUTY_TAIL                 20 // Helicopter hover duty
#define MAX_DUTY_TAIL                    97  // Minimum helicopter duty cycle
#define MIN_DUTY_TAIL                    2  // Minimum helicopter duty cycle
#define TIMER_RATE                       1000 // Determines rate of timer interrupt

/*
 * 0.6 pro, 0.0005 int - main
 */

//*****************************************************************************
// Intialise timer for PI control update
//*****************************************************************************
void
initResponseTimer (void);

//*****************************************************************************
// Pass PWM tail duty out of module
//*****************************************************************************
uint32_t
getTailDuty(void);

//*****************************************************************************
// Pass PWM main duty out of module
//*****************************************************************************
uint32_t
getMainDuty(void);

#endif /* RESPONSECONTROL_H_ */
