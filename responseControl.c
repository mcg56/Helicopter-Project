//
// responseControl.c -  PI control for helicotper control response
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Date 19/04/2021
//

#include <stdint.h>
#include <stdbool.h>
#include "stdlib.h"
#include "responseControl.h"


//*****************************************************************************
// Caculate helicopter main rotor response using PI control
//*****************************************************************************
int
dutyResponseMain(int16_t current_height, int16_t target_percent)
{
    int16_t duty_cycle;

    duty_cycle = PROPORTIONAL_GAIN_MAIN * (target_percent - current_height);

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY) {
        duty_cycle = MAX_DUTY;
    } else if (duty_cycle < MIN_DUTY) {
        duty_cycle = MIN_DUTY;
    }

    return duty_cycle;
}

//*****************************************************************************
// Caculate helicopter tail rotor response using PI control
//*****************************************************************************
int
dutyResponseTail(int16_t current_yaw, int16_t target_yaw)
{
    int16_t duty_cycle;

    duty_cycle = PROPORTIONAL_GAIN_TAIL * (target_yaw - current_yaw) + 20;

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY) {
        duty_cycle = MAX_DUTY;
    } else if (duty_cycle < MIN_DUTY) {
        duty_cycle = MIN_DUTY;
    }

    return duty_cycle;
}


