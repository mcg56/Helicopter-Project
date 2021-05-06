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

static int16_t prev_error_main;
static int16_t prev_error_tail;
static float integral_main;
static float integral_tail;

//*****************************************************************************
// Calculate helicopter main rotor response using PI control
//*****************************************************************************
int16_t
dutyResponseMain(int16_t current_height, int16_t target_percent)
{
    int16_t duty_cycle;
    float d_integral;
    int16_t delta_t = 1;
    int16_t error;
    int16_t proportional;

    error = target_percent - current_height;

    proportional = PROPORTIONAL_GAIN_MAIN * error;

    d_integral = INTEGRAL_GAIN_MAIN * (error - prev_error_main) * delta_t;

    duty_cycle = proportional + (integral_main + d_integral) + OFFSET_DUTY;

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY_MAIN) {
        duty_cycle = MAX_DUTY_MAIN;
    } else if (duty_cycle < MIN_DUTY_MAIN) {
        duty_cycle = MIN_DUTY_MAIN;
    } else {
        integral_main += d_integral;
    }

    prev_error_main = error;
    return duty_cycle;
}

//*****************************************************************************
// Caculate helicopter tail rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseTail(int16_t current_yaw, int16_t target_yaw)
{
    int32_t duty_cycle;
    float d_integral;
    int16_t delta_t = 1;
    int16_t error;
    int16_t proportional;

    error = target_yaw - current_yaw;

    proportional = PROPORTIONAL_GAIN_TAIL * error;

    d_integral = INTEGRAL_GAIN_TAIL * (error - prev_error_tail) * delta_t;

    duty_cycle = proportional + (integral_tail + d_integral);

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY_TAIL) {
        duty_cycle = MAX_DUTY_TAIL;
    } else if (duty_cycle < MIN_DUTY_TAIL) {
        duty_cycle = MIN_DUTY_TAIL;
    }

    prev_error_tail = error;
    return duty_cycle;
}

//*****************************************************************************
// The interrupt handler for the for Timer interrupt.
//*****************************************************************************
void
AltitudeControlIntHandler (void)
{
    // Clear the timer interrupt flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    pwm_main_duty = dutyResponseMain(height_percent, target_height_percent);
    Acount++;
    setPWMMain (PWM_MAIN_FREQ, pwm_main_duty);

}

//*****************************************************************************
// Intialise timer for PI control update
//*****************************************************************************
void
initAltTimer (void)
{
    //
    // The Timer0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //
    // Configure Timer0B as a 16-bit periodic timer.
    //
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000);

    TimerIntRegister(TIMER0_BASE, TIMER_A, AltitudeControlIntHandler);

    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER0_BASE, TIMER_A);
}

