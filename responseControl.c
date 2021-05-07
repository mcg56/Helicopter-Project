//
// responseControl.c -  PI control for helicotper control response
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Date 19/04/2021
//

#include <stdint.h>
#include "responseControl.h"
#include "driverlib/timer.h"
#include "yaw.h"
#include "altitude.h"
#include "pwmGen.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
//#include <stdbool.h>
//#include "stdlib.h"

//*****************************************************************************
// Global variables
//*****************************************************************************
// Integral control information
static int16_t prev_error_main;
static int16_t prev_error_tail;
static float integral_main;
static float integral_tail;

// Altitude data
static int16_t height_percent;
static int16_t target_height_percent;
static uint32_t pwm_main_duty;

// Yaw data
static int16_t yaw_degree;
static int16_t target_yaw;
static uint32_t pwm_tail_duty;

//*****************************************************************************
// Function prototypes
//*****************************************************************************
int32_t dutyResponseMain(int16_t current_height, int16_t target_percent);
int32_t dutyResponseTail(int16_t current_yaw, int16_t target_yaw);

//*****************************************************************************
// The interrupt handler for the for timer interrupt.
//*****************************************************************************
void
responseControlIntHandler (void)
{
    // Clear the timer interrupt flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Update data from altitude module
    height_percent = getAltitudeHeight();
    target_height_percent = getAltitudeTarget();

    // Calculate and set main rotor PWM using PI control
    pwm_main_duty = dutyResponseMain(height_percent, target_height_percent);
    setPWMMain (PWM_MAIN_FREQ, pwm_main_duty);

    // Updata data from yaw module
    yaw_degree = getYaw();
    target_yaw = getYawTarget();

    // Calculate and set tail rotor PWM using PI control
    pwm_tail_duty = dutyResponseTail(yaw_degree, target_yaw);
    setPWMTail (PWM_TAIL_FREQ, pwm_tail_duty);
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

    // Set timer value
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000);

    // Register interrupt
    TimerIntRegister(TIMER0_BASE, TIMER_A, responseControlIntHandler);

    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable timer
    TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
// Calculate helicopter main rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseMain(int16_t current_height, int16_t target_percent)
{
    float d_integral;
    int32_t duty_cycle;
    int16_t error;
    int16_t proportional;

    // Current height error
    error = target_percent - current_height;

    // Proportional response
    proportional = PROPORTIONAL_GAIN_MAIN * error;

    // Integral response for current time step
    d_integral = INTEGRAL_GAIN_MAIN * (error - prev_error_main);

    // Total response duty cycle
    duty_cycle = proportional + (integral_main + d_integral);

    // Limit duty cycle values
    if (duty_cycle > MAX_DUTY_MAIN) {
        duty_cycle = MAX_DUTY_MAIN;
    } else if (duty_cycle < MIN_DUTY_MAIN) {
        duty_cycle = MIN_DUTY_MAIN;
    } else {
        integral_main += d_integral;
    }

    // Update previous error for next time step
    prev_error_main = error;

    return duty_cycle;
}

//*****************************************************************************
// Calculate helicopter tail rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseTail(int16_t current_yaw, int16_t target_yaw)
{
    int32_t duty_cycle;
    float d_integral;
    int16_t error;
    int16_t proportional;

    // Current yaw error
    error = target_yaw - current_yaw;

    // Proportional response
    proportional = PROPORTIONAL_GAIN_TAIL * error;

    // Integral response for current time step
    d_integral = INTEGRAL_GAIN_TAIL * (error - prev_error_tail);

    // Total response duty cycle
    duty_cycle = proportional + (integral_tail + d_integral);

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY_TAIL) {
        duty_cycle = MAX_DUTY_TAIL;
    } else if (duty_cycle < MIN_DUTY_TAIL) {
        duty_cycle = MIN_DUTY_TAIL;
    } else {
        integral_tail += d_integral;
    }

    // Update previous error for next time step
    prev_error_tail = error;

    return duty_cycle;
}

//*****************************************************************************
// Pass PWM tail duty out of module
//*****************************************************************************
uint32_t
getTailDuty(void)
{
    return pwm_tail_duty;
}

//*****************************************************************************
// Pass PWM main duty out of module
//*****************************************************************************
uint32_t
getMainDuty(void)
{
    return pwm_main_duty;
}


