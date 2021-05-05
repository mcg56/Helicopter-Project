//*****************************************************************************
//
// Milestone_2.c - Measure and display helicopter altitude and yaw
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Date 19/04/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stdlib.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"
#include "buttons4.h"
#include "yaw.h"
#include "altitude.h"
#include "display.h"
#include "circBufT.h"
#include "pwmGen.h"
#include "uart.h"
#include "system.h"
#include "switches.h"


#include "responseControl.h"



int
main(void)
{
    flight_mode current_state;
    int32_t current_height;
    int32_t landed_height;
    uint32_t duty_main;
    uint32_t duty_tail;
    int16_t height_percent;
    int16_t target_height_percent;
    int16_t yaw_degree;
    int16_t target_yaw;
    uint8_t slowTick;
    bool reference_found = false;

    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_PWM);  // Main Rotor PWM
    SysCtlPeripheralReset (PWM_TAIL_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_TAIL_PERIPH_PWM);  // Tail Rotor PWM
    SysCtlPeripheralReset (LEFT_BUT_PERIPH);
    SysCtlPeripheralReset (UP_BUT_PERIPH);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralReset (UART_USB_PERIPH_UART);
    SysCtlPeripheralReset (UART_USB_PERIPH_GPIO);

    initClock ();
    initButtons ();
    initAltitude ();
    initYaw ();
    //initSysTick ();
    initDisplay ();
    initUSB_UART ();
    initSwitches ();
    initSoftReset ();

    // Enable interrupts to the processor.
    IntMasterEnable();
    // System delay for accurate initial value calibration
    SysCtlDelay (SysCtlClockGet() / 2);

    landed_height = getHeight();        // Set initial helicopter resting height


    while (1)
    {
        SysCtlDelay (SysCtlClockGet() / 32);  // Update display at approx 32 Hz

        current_state = switchValue();

        switch (current_state)
        {
        case landed:
            target_height_percent = 0;
            target_yaw = 0;
            // Motors stop when low enough
            // Disable switch funciton until landed
            break;
        case take_off:
            // Find reference yaw on first take off
            if (reference_found == false)
            {
                findReference();
                reference_found = true;

                yaw_degree = 0;
                target_yaw = 0;
            }
            // Increase main rotor duty cycle if up button pressed
            if ((checkButton (UP) == PUSHED) && (target_height_percent < 90))
            {
                target_height_percent += 10;
            }

            // Decrease main rotor duty cycle if down button pressed
            if (checkButton (DOWN) == PUSHED)
            {
                if (target_height_percent > 10) {
                    target_height_percent -= 10;
                } else {
                    target_height_percent = 0;
                }
            }

            // Increase yaw if left button pushed
            if ((checkButton (LEFT) == PUSHED))
            {
                target_yaw -= 15;
            }

            // Decrease yaw if right button pushed
            if ((checkButton (RIGHT) == PUSHED))
            {
                target_yaw += 15;
            }
            break;
        }

        // Get current helicopter height
        current_height = getHeight();

        // Convert ADC height to percentage
        height_percent = calculate_percent_height(current_height, landed_height);

        // Get yaw from yaw module
        yaw_degree = getYaw();

        // Display helicopter details
        displayMeanVal (height_percent, yaw_degree);

        // Update altitude control
        duty_main = updateAltitude(height_percent, target_height_percent);

        // Update yaw control
        duty_tail = updateYaw(yaw_degree, target_yaw);

        // Get slowTick from system module
        slowTick = getSlowTick();

        // Carry out UART transmission of helicopter data
        UARTTransData (height_percent, target_height_percent, yaw_degree, target_yaw, duty_main, duty_tail, current_state, slowTick);

    }
}

// To do:
// Add flight mode land functionality
// Remove functions from headers that don't need to be there
// Clean #includes
// Convert switches to interrupt on regular timer
// Add pi as interrupt off timer
// change to +180 to -180

// Questions?
// Location of flight state enum in switches?
// Is it ok to use prescalers for pwm signal with clock issues

