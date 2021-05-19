//*****************************************************************************
//
// main.c
//
// Top level file for a helicopter control system. The helicopter is able to take
// off and initialise, finding the hover point and orientating itself to a reference
// yaw, before user interface control allows the helicopter to change height and
// yaw. This control is done using the four push buttons on the TIVA board. Returning
// the switch to the off position puts the helicopter into a smooth landing
// state in the reference orientation position.
// Helicopter state, position and dut cycle information is continuously updated
// through both UART transmissions and via the OLED display.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified: 19/5/2021
//

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
    uint32_t hover_height = 1;
    uint8_t slowTick;
    bool ref_yaw_found = false;
    bool hover_duty_found = false;


    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (LEFT_BUT_PERIPH);
    SysCtlPeripheralReset (UP_BUT_PERIPH);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralReset (UART_USB_PERIPH_UART);
    SysCtlPeripheralReset (UART_USB_PERIPH_GPIO);
    SysCtlPeripheralReset (SYSCTL_PERIPH_TIMER0);

    // Initialise peripherals and modules
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
    SysCtlDelay (SysCtlClockGet() / 60);

    // Set initial helicopter resting height
    height_landed_adc = getHeight();

    // Intialise helicopter state
    current_state = landed;

    while (1)
    {
        // Pace program at apporx 50 Hz
        SysCtlDelay (SysCtlClockGet() / 50);

        // Update helicopter state
        current_state = updateState(current_state);

        // Helicopter functionality based on current state
        switch (current_state)
        {
        case landed:
            // Keep target values at home
            height_data.target = 0;
            yaw_data.target = 0;
            break;
        case landing:
            // Set target values to home
            height_data.target = 0;
            yaw_data.target = 0;

            // Update helicopter state to landed when reference orientation reached
            if (yaw_data.current == 0 && height_data.current <= 0) {
                current_state = landed;
            }
            break;
        case initialising:
            // First find hover duty for the helicopter
            if (!hover_duty_found) {
                height_data.target = hover_height;
                if (height_data.current == hover_height) {
                    hover_duty_found = true;
                    height_data.target = 0;
                }

            // Second find the refence yaw orientation
            } else if (!ref_yaw_found) {
                // Update the reference yaw value from yaw module
                ref_yaw_found = findReference();

            // Once complete set the mode to flying
            } else {
                current_state = flying;
                height_data.target = 0;
            }
            break;
        case flying:
            // Increase main rotor duty cycle if up button pressed
            if (checkButton (UP) == PUSHED)
            {
                // Limit max height to 100%
                if (height_data.target < 90) {
                    height_data.target += 10;
                } else {
                    height_data.target = 100;
                }
            }

            // Decrease main rotor duty cycle if down button pressed
            if (checkButton (DOWN) == PUSHED)
            {
                // Limit minimum height to 0%
                if (height_data.target > 10) {
                    height_data.target -= 10;
                } else {
                    height_data.target = 0;
                }
            }

            // Decrease yaw if left button pushed
            if ((checkButton (LEFT) == PUSHED))
            {
                // Keep yaw in 180 to -179 range
                if (yaw_data.target == -165) {
                    yaw_data.target = 180;
                } else {
                    yaw_data.target -= 15;
                }
            }

            // Increase yaw if right button pushed
            if ((checkButton (RIGHT) == PUSHED))
            {
                // Keep yaw in 180 to -179 range
                if (yaw_data.target == 180) {
                    yaw_data.target = -165;
                } else {
                    yaw_data.target += 15;
                }
            }
        }

        // Get current helicopter height
        height_current_adc = getHeight();

        // Convert ADC height to percentage
        height_data.current = calculate_percent_height(height_current_adc, height_landed_adc);

        // Get current yaw from yaw module
        yaw_data.current = getYawCurrent();

        // Update response control
        updateResponseControl(height_data, yaw_data);

        // Update helicopter duty cycle values
        heli_duty = getHeliDuty();

        // Display helicopter details
        displayData (height_data.current, yaw_data.current, heli_duty);

        // Get slowTick from system module
        slowTick = getSlowTick();

        // Carry out UART transmission of helicopter data
        UARTTransData (height_data, yaw_data, heli_duty, current_state, slowTick);
    }
}
