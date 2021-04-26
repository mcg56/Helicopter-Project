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
// Caculate helicopter rotor response using PI control
//*****************************************************************************
extern int
dutyResponse(int16_t current_height, int16_t target_percent)
{
    int16_t duty_cycle;

    duty_cycle = PROPORTIONAL_GAIN * (target_percent - current_height);

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY) {
        duty_cycle = MAX_DUTY;
    } else if (duty_cycle < MIN_DUTY) {
        duty_cycle = MIN_DUTY;
    }

    return duty_cycle;
}


