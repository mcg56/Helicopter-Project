//*****************************************************************************
//
// Milestone_2.c - Measure and display helicopter altitude and yaw
//
// Authors: Tom Peterson, Matt Comber, Mark Gardyne
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


//*****************************************************************************
// The interrupt handler for the for SysTick interrupt.
//*****************************************************************************
void
SysTickIntHandler(void)
{
    // Poll the buttons
    updateButtons();

    ADCProcessorTrigger(ADC0_BASE, 3);

}

//*****************************************************************************
// Initialisation functions for the clock
// Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
void
initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}


//*************************************************************
// SysTick interrupt
// Sourced from:  P.J. Bones  UCECE
//*************************************************************
void
initSysTick (void)
{
    //
    // Set up the period for the SysTick timer.  The SysTick
    // timer period is set as a function of the system clock.
    SysTickPeriodSet (SysCtlClockGet() / SYSTICK_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister (SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable ();
    SysTickEnable ();
}


int
main(void)
{
    int32_t mean_height;
    int32_t landed_height;
    int32_t display_deg;
    int32_t height_percent;
    displayType display_state;

    SysCtlPeripheralReset (LEFT_BUT_PERIPH);
    SysCtlPeripheralReset (UP_BUT_PERIPH);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB);

    initClock ();
    initButtons ();
    initAltitude ();
    initSysTick ();
    initDisplay ();
    initGPIOPins ();

    // Enable interrupts to the processor.
    IntMasterEnable();
    // System delay for accurate initial value calibration
    SysCtlDelay (SysCtlClockGet() / 2);

    landed_height = getHeight();        // Set initial helicopter resting height
    display_state = percent_height;     // Set initial display state to percentage

    while (1)
    {
        SysCtlDelay (SysCtlClockGet() / 32);  // Update display at approx 32 Hz

        // Reset landed helicopter height if left button pushed
        if ((checkButton (LEFT) == PUSHED))
        {
            landed_height = getHeight();
        }

        // Update height display method with UP button
        if ((checkButton (UP) == PUSHED))
        {
            if (display_state == off) {
                display_state = percent_height;
            } else {
                display_state++;
            }
        }

        // Get current helicopter height
        mean_height = getHeight();

        height_percent = calculate_percent_height(mean_height, landed_height);

        // Display helicopter height
        display_deg = getYaw();
        displayMeanVal (mean_height, height_percent, display_state, display_deg);
    }
}

// Ask about typedef of enum for display state
// How to divide with float (scale correction) and display
// Is it ok call update yaw in interrupt?
// What is reasonable drift for yaw
