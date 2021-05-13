//*****************************************************************************
//
// Milestone_2.c - Measure and display helicopter altitude and yaw
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Date 19/04/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "buttons4.h"
#include "yaw.h"
#include "altitude.h"
#include "display.h"
#include "uart.h"
#include "system.h"
#include "switches.h"


int
main(void)
{
    flight_mode current_state;
    height_data_s height_data;
    yaw_data_s yaw_data;
    int32_t current_height;
    int32_t landed_height;
    uint32_t duty_main;
    uint32_t duty_tail;
    uint8_t slowTick;
    bool reference_found = false;

    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (LEFT_BUT_PERIPH);
    SysCtlPeripheralReset (UP_BUT_PERIPH);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralReset (UART_USB_PERIPH_UART);
    SysCtlPeripheralReset (UART_USB_PERIPH_GPIO);
    SysCtlPeripheralReset (SYSCTL_PERIPH_TIMER0);

    initClock ();
    initAltitude ();
    initYaw ();
    initButtons ();
    initDisplay ();
    initUSB_UART ();
    initSwitches ();
    initSoftReset ();

    // Enable interrupts to the processor.
    IntMasterEnable();

    // System delay for accurate initial value calibration
    SysCtlDelay (SysCtlClockGet() / 2);

    // Set initial helicopter resting height
    landed_height = getHeight();

    // Intialise helicopter state
    current_state = landed;

    while (1)
    {
        SysCtlDelay (SysCtlClockGet() / 32);  // Update display at approx 32 Hz

        // Update helicopter state
        current_state = updateState(current_state);

        switch (current_state)
        {
        case landed:
            height_data.target = 0;
            break;
        case landing:
            // Set target values to home
            height_data.target = 0;
            yaw_data.target = 0;

            // Update state to landed when targets reached
            if (((yaw_data.current > 340) || (yaw_data.current <= 20)) && height_data.current == 0) {
                current_state = landed;
            }
            break;
        case flying:
            // Find reference yaw on first take off and prevent button usage
            if (reference_found == false)
            {
                reference_found = findReference();
            } else {
                // Increase main rotor duty cycle if up button pressed
                if (checkButton (UP) == PUSHED)
                {
                    if (height_data.target < 90) {
                        height_data.target += 10;
                    } else {
                        height_data.target = 100;
                    }
                }

                // Decrease main rotor duty cycle if down button pressed
                if (checkButton (DOWN) == PUSHED)
                {
                    if (height_data.target > 10) {
                        height_data.target -= 10;
                    } else {
                        height_data.target = 0;
                    }
                }

                // Decrease yaw if left button pushed
                if ((checkButton (LEFT) == PUSHED))
                {
                    if (yaw_data.target == 0) {
                        yaw_data.target = 345;
                    } else {
                        yaw_data.target -= 15;
                    }
                }

                // Increase yaw if right button pushed
                if ((checkButton (RIGHT) == PUSHED))
                {
                    if (yaw_data.target == 345) {
                        yaw_data.target = 0;
                    } else {
                        yaw_data.target += 15;
                    }
                }
            }
        }

        // Get current helicopter height
        current_height = getHeight();

        // Convert ADC height to percentage
        height_data.current = calculate_percent_height(current_height, landed_height);

        // Get yaw from yaw module
        yaw_data.current = getYawCurrent();

        // Update altitude module data
        duty_main = updateAltitude(height_data);

        // Update yaw module data
        duty_tail = updateYaw(yaw_data);

        // Display helicopter details
        displayMeanVal (height_data.current, yaw_data.current, duty_main, duty_tail);

        // Get slowTick from system module
        slowTick = getSlowTick();

        // Carry out UART transmission of helicopter data
        UARTTransData (height_data, yaw_data, duty_main, duty_tail, current_state, slowTick);
    }
}

// To do:
// Change data transfer to struct
// Reduce response interrupt size


// Questions?
// How to choose timer value

