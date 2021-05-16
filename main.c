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
#include "flight_mode.h"
#include "responseControl.h"


int
main(void)
{
    flight_mode current_state;
    height_data_s height_data;
    yaw_data_s yaw_data;
    duty_cycle_s heli_duty;
    int32_t height_current_adc;
    int32_t height_landed_adc;
    uint8_t slowTick;
    bool ref_yaw_found = false;
    bool hover_duty_found = false;
    uint32_t hover_height = 2;

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
    height_landed_adc = getHeight();

    // Intialise helicopter state
    current_state = landed;

    while (1)
    {
        // Update display at approx 32 Hz
        SysCtlDelay (SysCtlClockGet() / 32);

        // Update helicopter state
        current_state = updateState(current_state);

        // Helicopter functionality based on current state
        switch (current_state)
        {
        case landed:
            height_data.target = 0;
            break;
        case landing:
            // Set target values to home
            height_data.target = 0;
            yaw_data.target = 0;

            //((yaw_data.current > 355) || (yaw_data.current <= 5))
            // Update state to landed when targets reached
            if (yaw_data.current == 0 && height_data.current == 0) {
                current_state = landed;
            }
            break;
        case initialising:
            // Find reference yaw on first take off and prevent button usage
            if (!ref_yaw_found) {
                ref_yaw_found = findReference();
            } else if (!hover_duty_found) {
                if (height_data.current == hover_height) {
                    hover_duty_found = true;
                }
                height_data.target = hover_height;
            } else {
                current_state = flying;
            }
            break;
        case flying:
            // Increase main rotor duty cycle if up button pressed
            if (checkButton (UP) == PUSHED)
            {
                if (height_data.target == hover_height) {
                    height_data.target = 10;
                } else if (height_data.target < 90) {
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
                    height_data.target = hover_height;
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

        // Get current helicopter height
        height_current_adc = getHeight();

        // Convert ADC height to percentage
        height_data.current = calculate_percent_height(height_current_adc, height_landed_adc);

        // Get yaw from yaw module
        yaw_data.current = getYawCurrent();

        // Update response control
        updateResponseControl(height_data, yaw_data);

        // Update helicopter duty cycles
        heli_duty = getHeliDuty();

        // Display helicopter details
        displayMeanVal (height_data.current, yaw_data.current, heli_duty);

        // Get slowTick from system module
        slowTick = getSlowTick();

        // Carry out UART transmission of helicopter data
        UARTTransData (height_data, yaw_data, heli_duty, current_state, slowTick);
    }
}
