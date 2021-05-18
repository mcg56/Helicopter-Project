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
#include "flight_mode.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"


//*****************************************************************************
// Global variables
//*****************************************************************************
// Integral control information
static float integral_main;
static float integral_tail;

// Altitude data
static height_data_s height_data;
static uint32_t height_sweep_duty = 30;
static uint32_t offset_duty_main = 30; // Helicopter hover duty

// Yaw data
static yaw_data_s yaw_data;
static uint32_t yaw_sweep_duty = 50;

// Current helicopter state
flight_mode current_state;
bool PI_control_enable = false;
bool hover_duty_found = false;

duty_cycle_s heli_duty;

//*****************************************************************************
// Function prototypes
//*****************************************************************************
int32_t dutyResponseMain();
int32_t dutyResponseTail();

//*****************************************************************************
// The interrupt handler for the for timer interrupt.
//*****************************************************************************
void
responseControlIntHandler (void)
{
    // Clear the timer interrupt flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    if (PI_control_enable) {
        // Calculate main rotor PWM using PI control
        heli_duty.main = dutyResponseMain();

        // Calculate tail rotor PWM using PI control
        heli_duty.tail = dutyResponseTail();

        // Set duty values
        setPWMTail (PWM_TAIL_FREQ, heli_duty.tail);
        setPWMMain (PWM_MAIN_FREQ, heli_duty.main);
    }

}

//*****************************************************************************
// Intialise timer for PI control update
//*****************************************************************************
void
initResponseTimer (void)
{
    // The Timer0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Configure Timer0B as a 16-bit periodic timer.
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // Set timer value
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / TIMER_RATE);

    // Register interrupt
    TimerIntRegister(TIMER0_BASE, TIMER_A, responseControlIntHandler);

    // Configure the Timer0B interrupt for timer timeout.
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable timer
    TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
// Update PWM based on helicopter state
//*****************************************************************************
void
updateResponseControl (height_data_s height_data_in, yaw_data_s yaw_data_in)
{
    // Update helicopter state
     current_state = getState();

     // Update state data
     height_data = height_data_in;
     yaw_data = yaw_data_in;

     switch (current_state)
     {
     case landed:
         // Turn off rotors
         PI_control_enable = false;
         heli_duty.tail = 0;
         heli_duty.main = 0;
         integral_main = 0;
         integral_tail = 0;
         setPWMMain (PWM_MAIN_FREQ, heli_duty.main);
         setPWMTail (PWM_TAIL_FREQ, heli_duty.tail);
         break;
     case initialising:
         // Find hover duty cycle
         if (!hover_duty_found) {
             PI_control_enable = true;
             if (height_data.current == height_data.target) {
                 hover_duty_found = true;
                 offset_duty_main = heli_duty.main;
                 height_sweep_duty = heli_duty.main - 10;
             }
         }

         // Find reference yaw
         if (refFound() && hover_duty_found) {
             PI_control_enable = true;
             integral_main = 0;
             integral_tail = 0;
         } else if (hover_duty_found) {
             PI_control_enable = false;

             // Set duty to sweeping values during intialisation
             heli_duty.tail = yaw_sweep_duty;
             heli_duty.main = height_sweep_duty;

             // Set duty values
             setPWMTail (PWM_TAIL_FREQ, heli_duty.tail);
             setPWMMain (PWM_MAIN_FREQ, heli_duty.main);
         }
         break;
     case landing:
         // Use same control as take off
         break;
     case flying:
         PI_control_enable = true;
     }
}

//*****************************************************************************
// Calculate helicopter main rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseMain()
{
    int duty_cycle;
    float step_integral;
    float error;
    float proportional;

    // Current height error
    error = height_data.target - height_data.current;

    // Proportional response
    proportional = PROPORTIONAL_GAIN_MAIN * error;

    // Integral response for current time step
    step_integral = INTEGRAL_GAIN_MAIN * error;

    // Total response duty cycle
    duty_cycle = proportional + (integral_main + step_integral) + offset_duty_main;

    // Limit duty cycle values
    if (duty_cycle > MAX_DUTY_MAIN) {
        duty_cycle = MAX_DUTY_MAIN;
    } else if (duty_cycle < MIN_DUTY_MAIN) {
        duty_cycle = MIN_DUTY_MAIN;
    } else {
        integral_main += step_integral;
    }

    return duty_cycle;
}

//*****************************************************************************
// Calculate helicopter tail rotor response using PI control
//*****************************************************************************
int32_t
dutyResponseTail()
{
    uint32_t duty_cycle;
    float step_integral;
    int16_t error;
    int16_t proportional;
    int16_t full_rot = 360;    // Degrees in full rotation
    int16_t half_rot = 180;    // Half rotation

    // Current yaw error accounting for -179 to 180 degree range
    if (yaw_data.current < 0 && (yaw_data.target > (yaw_data.current + half_rot)))  {
        error = -1 * (full_rot - (yaw_data.target - yaw_data.current));
    } else if (yaw_data.current > 0 && (yaw_data.target < (yaw_data.current - half_rot))) {
        error = (full_rot + (yaw_data.target - yaw_data.current));
    } else {
        error = yaw_data.target - yaw_data.current;
    }

    // Proportional response
    proportional = PROPORTIONAL_GAIN_TAIL * error;

    // Integral response for current time step
    step_integral = INTEGRAL_GAIN_TAIL * error;

    // Total response duty cycle
    duty_cycle = proportional + (integral_tail + step_integral) + COUPLING_RATIO;

    //Limit duty cycle values
    if (duty_cycle > MAX_DUTY_TAIL) {
        duty_cycle = MAX_DUTY_TAIL;
    } else if (duty_cycle < MIN_DUTY_TAIL) {
        duty_cycle = MIN_DUTY_TAIL;
    } else {
        integral_tail += step_integral;
    }

    return duty_cycle;
}

//*****************************************************************************
// Pass PWM main and tail duty out of module
//*****************************************************************************
duty_cycle_s
getHeliDuty(void)
{
    return heli_duty;
}
