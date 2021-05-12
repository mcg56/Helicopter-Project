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
#include "switches.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"


//*****************************************************************************
// Global variables
//*****************************************************************************
// Integral control information
static float integral_main;
static float integral_tail;

// Altitude data
static float height_percent;
static float target_height_percent;
static float pwm_main_duty;
static float height_sweep_duty = 30;

// Yaw data
static int16_t yaw_degree;
static int16_t target_yaw;
static uint32_t pwm_tail_duty;
static uint32_t yaw_sweep_duty = 60;

// Current helicopter state
flight_mode current_state;

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

    // Update helicopter state
    current_state = getState();

    switch (current_state)
    {
    case landed:
        // Turn off rotors
        pwm_tail_duty = 0;
        pwm_main_duty = 0;
        setPWMTail (PWM_TAIL_FREQ, pwm_tail_duty);
        setPWMMain (PWM_MAIN_FREQ, pwm_main_duty);
        break;
    case landing:
        // Use same control as take off so continue
    case take_off:
        // Initiate PI control is initialisation finished
        if (refFound()) {
            // Update data from altitude module
            height_percent = getAltitudeHeight();
            target_height_percent = getAltitudeTarget();

            // Calculate and set main rotor PWM using PI control
            pwm_main_duty = dutyResponseMain(height_percent, target_height_percent);

            // Update data from yaw module
            yaw_degree = getYaw();
            target_yaw = getYawTarget();

            // Calculate and set tail rotor PWM using PI control
            pwm_tail_duty = dutyResponseTail(yaw_degree, target_yaw);
        } else {
            // Set duty to sweeping values during intialisation
            pwm_tail_duty = yaw_sweep_duty;
            pwm_main_duty = height_sweep_duty;
        }

        // Set duty values
        setPWMTail (PWM_TAIL_FREQ, pwm_tail_duty);
        setPWMMain (PWM_MAIN_FREQ, pwm_main_duty);
    }
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
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / TIMER_RATE);

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
    int duty_cycle;
    float error;
    float proportional;

    // Current height error
    error = target_percent - current_height;

    // Proportional response
    proportional = PROPORTIONAL_GAIN_MAIN * error;

    // Integral response for current time step
    d_integral = INTEGRAL_GAIN_MAIN * (error);

    // Total response duty cycle
    duty_cycle = proportional + (integral_main + d_integral) + OFFSET_DUTY_MAIN;

    // Limit duty cycle values
    if (duty_cycle > MAX_DUTY_MAIN) {
        duty_cycle = MAX_DUTY_MAIN;
    } else if (duty_cycle < MIN_DUTY_MAIN) {
        duty_cycle = MIN_DUTY_MAIN;
    } else {
        integral_main += d_integral;
    }

    return duty_cycle;
}

//*****************************************************************************
// Calculate helicopter tail rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseTail(int16_t current_yaw, int16_t target_yaw)
{
    uint32_t duty_cycle;
    float d_integral;
    int16_t error;
    int16_t proportional;
    int16_t full_rot = 360;    // Degrees in full rotation
    int16_t half_rot = 180;    // Half rotation

    // Current yaw error accounting for 0 to 360 degree range
    if (current_yaw > 180 && (target_yaw < (current_yaw - 180)))  {
        error = (360 - (current_yaw - target_yaw));
    } else if (current_yaw < 180 && (target_yaw > (current_yaw + 180))) {
        error = -1 * (360 + (current_yaw - target_yaw));
    } else {
        error = target_yaw - current_yaw;
    }


    // Proportional response
    proportional = PROPORTIONAL_GAIN_TAIL * error;

    // Integral response for current time step
    d_integral = INTEGRAL_GAIN_TAIL * (error);

    // Total response duty cycle
    duty_cycle = proportional + (integral_tail + d_integral) + OFFSET_DUTY_TAIL;

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY_TAIL) {
        duty_cycle = MAX_DUTY_TAIL;
    } else if (duty_cycle < MIN_DUTY_TAIL) {
        duty_cycle = MIN_DUTY_TAIL;
    } else {
        integral_tail += d_integral;
    }

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
