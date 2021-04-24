//*****************************************************************************
//
// Milestone_2.c - Measure and display helicopter altitude and yaw
//
// Authors: Tom Peterson, Matt Comber, Mark Gardyne
// Date 19/04/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)

#include <stdint.h> /
#include <stdio.h>  /
#include <stdbool.h>  /
#include "stdlib.h"  /
#include "inc/hw_memmap.h"  /
#include "inc/hw_types.h"  /
#include "inc/hw_ints.h"    /
#include "driverlib/adc.h"  /
#include "driverlib/gpio.h" /
#include "driverlib/sysctl.h"   /
#include "driverlib/systick.h"  /
#include "driverlib/interrupt.h"    /
#include "driverlib/debug.h"    /
#include "driverlib/pin_map.h"  /
#include "utils/ustdlib.h"  /

#include "buttons4.h"   /


//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;        // Counter for the interrupts


//*****************************************************************************
// Function handles
//*****************************************************************************





//*****************************************************************************
// The interrupt handler for the for SysTick interrupt.
//*****************************************************************************
void
SysTickIntHandler(void)
{
    // Poll the buttons
    updateButtons();

    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;
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
    uint16_t i;
    int32_t sum;
    int32_t mean;
    int32_t landed_height;
    displayType display_state;

    SysCtlPeripheralReset (LEFT_BUT_PERIPH);
    SysCtlPeripheralReset (UP_BUT_PERIPH);
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB);

    initClock ();
    initButtons ();
    initCircBuf (&g_inBuffer, BUF_SIZE);
    initADC ();
    initButtons ();
    initSysTick ();
    initDisplay ();
    initGPIOPins ();

    // Enable interrupts to the processor.
    IntMasterEnable();
    // System delay for accurate initial value calibration
    SysCtlDelay (SysCtlClockGet() / 2);

    landed_height = calibrate_height(); // Set initial helicopter resting height
    display_state = percent_height;                  // Set initial display state to percentage
    yaw = 0;                            // Initialise yaw to zero;


    while (1)
    {
        SysCtlDelay (SysCtlClockGet() / 32);  // Update display at approx 32 Hz

        // Reset landed helicopter height if left button pushed
        if ((checkButton (LEFT) == PUSHED))
        {
            landed_height = calibrate_height();
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

        //
        // Background task: calculate the (approximate) mean of the values in the
        // circular buffer and display it, together with the sample number.
        sum = 0;
        for (i = 0; i < BUF_SIZE; i++)
            sum = sum + (readCircBuf (&g_inBuffer));

        // Calculate the rounded mean of the buffer contents
        mean = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;


        // Display helicopter height
        displayMeanVal (mean, landed_height, display_state);

    }
}

// Ask about typedef of enum for display state
// How to divide with float (scale correction) and display
// Is it ok call update yaw in interrupt?
