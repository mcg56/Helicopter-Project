//*****************************************************************************
//
// responseControl.c
//
// Motion control for helicopter. Takes current helicopter state, position and
// target position from the main and drives the rotors appropriately.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified: 19/5/2021
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
static float integral_main;     // Cumulative main integral value
static float integral_tail;     // Cumulative tail integral value

// PI gains
static float integral_gain_main = 0.00009;   // Integral control gain for main rotor
static float integral_gain_tail = 0.0000375; // Integral control gain for tail rotor
static float proportional_gain_main = 0.65;  // Proportional control gain for main rotor
static float proportional_gain_tail = 0.6;   // Proportional control gain for tail rotor

// Altitude data
static height_data_s height_data;       // Height current and target values
static uint32_t height_sweep_duty = 30; // Main duty for reference orientation sweep
static uint32_t offset_duty_main = 30;  // Helicopter hover duty

// Yaw data
static yaw_data_s yaw_data;             // Yaw current and target values
static uint32_t yaw_sweep_duty = 50;    // Tail duty for reference orientation sweep

// Current helicopter state
flight_mode current_state;              // Current helicopter state
bool PI_main_enable = false;            // PI main enable
bool PI_tail_enable = false;            // PI tail enable
bool hover_duty_found = false;          // Hover duty found flag

// Helicopter duty cycle
duty_cycle_s heli_duty;

// Counter used for gradual duty cycle decrement during landing
int landing_count = 0;

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

    // PI control for main rotor
    if (PI_main_enable) {

        // Calculate main rotor PWM using PI control
        heli_duty.main = dutyResponseMain();

        // Set duty value
        setPWMMain (PWM_MAIN_FREQ, heli_duty.main);
    }

    // PI control for tail rotor
    if (PI_tail_enable) {

        // Calculate tail rotor PWM using PI control
        heli_duty.tail = dutyResponseTail();

        // Set duty value
        setPWMTail (PWM_TAIL_FREQ, heli_duty.tail);
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

     // Update PWM signals using state approriate method
     switch (current_state)
     {
     case landed:
         // Turn off rotors
         PI_main_enable = false;
         PI_tail_enable = false;
         heli_duty.tail = 0;
         heli_duty.main = 0;
         setPWMMain (PWM_MAIN_FREQ, heli_duty.main);
         setPWMTail (PWM_TAIL_FREQ, heli_duty.tail);
         break;
     case initialising:

         // Find hover duty cycle
         if (!hover_duty_found) {
             // Increase integral constant for temporary faster wind up
             integral_gain_main = 0.001;

             // Enable PI control
             PI_main_enable = true;
             PI_tail_enable = true;

             // Update hover duty found when at hover point
             if (height_data.current == height_data.target) {
                 hover_duty_found = true;

                 // Set offset value
                 offset_duty_main = heli_duty.main;

                 // Calculate yaw sweep duty based on offset
                 yaw_sweep_duty = offset_duty_main + 15;

                 // Limit yaw sweep to maximum value
                 if (yaw_sweep_duty > MAX_DUTY_TAIL) {
                     yaw_sweep_duty = MAX_DUTY_TAIL;
                 }
             }
         }

         // Find reference yaw once hover point found
         if (refFound() && hover_duty_found) {
             // Reset main integral constant
             integral_gain_main = 0.0001;

             //Enable PI control
             PI_main_enable = true;
             PI_tail_enable = true;

             // Reset cumulative integral values
             integral_main = 0;
             integral_tail = 0;
         } else if (hover_duty_found) {
             // Disable PI control
             PI_main_enable = false;
             PI_tail_enable = false;

             // Set duty to sweeping values during intialisation
             heli_duty.tail = yaw_sweep_duty;
             heli_duty.main = height_sweep_duty;

             // Set duty values
             setPWMTail (PWM_TAIL_FREQ, heli_duty.tail);
             setPWMMain (PWM_MAIN_FREQ, heli_duty.main);
         }
         break;
     case landing:
         // Disable PI control for only main rotor
         PI_main_enable = false;

         // Decrement main duty gradually, while at correct yaw orientation, until at minimum value for smooth decent
         landing_count++;
         if (landing_count >= 5 && heli_duty.main > (offset_duty_main - 10) && yaw_data.current > -5 && yaw_data.current < 5) {
             heli_duty.main = heli_duty.main - 1;
             landing_count = 0;
         }

         // Update main duty value
         setPWMMain (PWM_MAIN_FREQ, heli_duty.main);
         break;
     case flying:
         // Allow full PI control
         PI_main_enable = true;
         PI_tail_enable = true;
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
    proportional = proportional_gain_main * error;

    // Integral response for current time step
    step_integral = integral_gain_main * error;

    // Total response duty cycle
    duty_cycle = proportional + (integral_main + step_integral) + offset_duty_main;

    // Limit duty cycle values and prevent integral windup
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

    // Current yaw error for shortest rotation direction, accounting for -179 to 180 degree range
    if (yaw_data.current < 0 && (yaw_data.target > (yaw_data.current + half_rot)))  {
        error = -1 * (full_rot - (yaw_data.target - yaw_data.current));
    } else if (yaw_data.current > 0 && (yaw_data.target < (yaw_data.current - half_rot))) {
        error = (full_rot + (yaw_data.target - yaw_data.current));
    } else {
        error = yaw_data.target - yaw_data.current;
    }

    // Proportional response
    proportional = proportional_gain_tail * error;

    // Integral response for current time step
    step_integral = integral_gain_tail * error;

    // Total response duty cycle
    duty_cycle = proportional + (integral_tail + step_integral) + COUPLING_OFFSET;

    // Limit duty cycle values and prevent integral windup
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
// Pass PWM main and tail duties out of module
//*****************************************************************************
duty_cycle_s
getHeliDuty(void)
{
    return heli_duty;
}
