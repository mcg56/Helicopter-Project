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
#include "driverlib/timer.h"
#include "yaw.h"
#include "altitude.h"
#include "pwmGen.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"

static int16_t prev_error_main;
static int16_t prev_error_tail;
static float integral_main;
static float integral_tail;

static int16_t height_percent;
static int16_t target_height_percent;
static uint32_t pwm_main_duty;


static int16_t yaw_degree;
static int16_t target_yaw;
static uint32_t pwm_tail_duty;


//*****************************************************************************
// The interrupt handler for the for Timer interrupt.
//*****************************************************************************
void
responseControlIntHandler (void)
{
    // Clear the timer interrupt flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    height_percent = getAltitudeData();
    target_height_percent = getAltitudeDataTarget();

    pwm_main_duty = dutyResponseMain(height_percent, target_height_percent);
    setPWMMain (PWM_MAIN_FREQ, pwm_main_duty);


    yaw_degree = getYawData();
    target_yaw = getYawDataTarget();

    pwm_tail_duty = dutyResponseTail(yaw_degree, target_yaw);
    setPWMTail (PWM_TAIL_FREQ, pwm_tail_duty);

    countbla = target_yaw; //Test point
}

//*****************************************************************************
// Intialise timer for PI control update
//*****************************************************************************
void
initResponseTimer (void)
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

    TimerIntRegister(TIMER0_BASE, TIMER_A, responseControlIntHandler);

    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
// Calculate helicopter main rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseMain(int16_t current_height, int16_t target_percent)
{
    int32_t duty_cycle;
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
    int16_t error;
    int16_t proportional;

    error = target_yaw - current_yaw;

    proportional = PROPORTIONAL_GAIN_TAIL * error;

    d_integral = INTEGRAL_GAIN_TAIL * (error - prev_error_tail);

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

uint32_t
getTailDuty(void)
{
    return pwm_tail_duty;
}

uint32_t
getMainDuty(void)
{
    return pwm_main_duty;
}


