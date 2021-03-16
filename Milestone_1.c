//*****************************************************************************
//
// Milestone_1.c - Simple voltage measure of helicopter altitude
//
// Authors: Mark Gardyne, Tom Peterson
//
// Code Sourced from:  P.J. Bones  UCECE
// Last modified:   16.3.21
//

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"
#include "stdlib.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define BUF_SIZE 10
#define SAMPLE_RATE_HZ 10
#define SYSTICK_RATE_HZ    100 // Systick configuration

//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;        // Counter for the interrupts

//*****************************************************************************
//
// The interrupt handler for the for SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Initiate a conversion
    //

    // Poll the buttons
    updateButtons();

    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;

    updateButtons(); // Poll the buttons
}

//*****************************************************************************
//
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
//
//*****************************************************************************
void
ADCIntHandler(void)
{
    uint32_t ulValue;

    //
    // Get the single sample from ADC0.  ADC_BASE is defined in
    // inc/hw_memmap.h
    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
    //
    // Place it in the circular buffer (advancing write index)
    writeCircBuf (&g_inBuffer, ulValue);
    //
    // Clean up, clearing the interrupt
    ADCIntClear(ADC0_BASE, 3);
}

//*****************************************************************************
// Initialisation functions for the clock (incl. SysTick), ADC, display
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

void
initADC (void)
{
    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);

    //
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}

void
initDisplay (void)
{
    // intialise the Orbit OLED display
    OLEDInitialise ();
}

/*************************************************************
 * SysTick interrupt
 ************************************************************/
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


//*****************************************************************************
//
// Function to calculate the helicopter height as a percent of the voltage range.
//
//*****************************************************************************
int
calculate_percent_height(uint16_t current_height, uint16_t landed_height)
{
    int16_t height_percent;
    int16_t ADC_bits = 4095; // 12 bit ADC
    float height_voltage_range = 0.8; // Voltage range over helicopter height
    float ADC_voltage_range = 3.33; // Total voltage range of ADC

    height_percent = 100 * ((landed_height - current_height)/((height_voltage_range/ADC_voltage_range)*ADC_bits)); // Calculate helicopter height as percentage

    return height_percent;

}

//*****************************************************************************
//
// Function to update the display with given string parameters and variables
//
//*****************************************************************************
void
displayUpdate (char *str1, char *str2, uint16_t num, uint8_t charLine, char *unit)
{
    char text_buffer[17];           //Display fits 16 characters wide.

    // "Undraw" the previous contents of the line to be updated.
    OLEDStringDraw ("                ", 0, charLine);
    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf(text_buffer, sizeof(text_buffer), "%s %s %3d%s", str1, str2, num, unit);
    // Update line on display.
    OLEDStringDraw (text_buffer, 0, charLine);
}

//*****************************************************************************
//
// Function to display either the mean ADC value (10-bit value, note), the helicopter height as a percentage
// or no display at all.
//
//*****************************************************************************
void
displayMeanVal(uint16_t meanVal, uint16_t landed_height)
{
    char string[17];  // 16 characters across the display
    int16_t height_percent;
    height_percent = calculate_percent_height(meanVal, landed_height);

    OLEDStringDraw ("Milestone 1", 0, 0);
    usnprintf (string, sizeof(string), "Height %5d%%", height_percent);
    OLEDStringDraw (string, 0, 1);


    // Test code to be removed
    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf (string, sizeof(string), "Mean ADC = %4d", meanVal);
    // Update line on display.
    OLEDStringDraw (string, 0, 2);

    displayUpdate ("Landed","", landed_height, 3, "");


}

//*****************************************************************************
//
// Function to find the initial voltage reading and record this as the landed helicopter height.
//
//*****************************************************************************
int
calibrate_height()
{
    int16_t start_height;
    int16_t sum;
    uint16_t i;

    sum = 0;
    for (i = 0; i < BUF_SIZE; i++)
        sum = sum + (readCircBuf (&g_inBuffer));
    // Calculate and display the rounded mean of the buffer contents
    start_height = ((2 * sum + BUF_SIZE) / 2 / BUF_SIZE);

   return start_height;
}

int
main(void)
{
    uint16_t i;
    int32_t sum;
    int16_t mean;
    int16_t landed_height;

    SysCtlPeripheralReset (LEFT_BUT_PERIPH);      // LEFT button GPIO

    initClock ();
    initButtons ();
    initADC ();
    initCircBuf (&g_inBuffer, BUF_SIZE);
    initButtons ();
    initSysTick ();
    initDisplay ();


    //
    // Enable interrupts to the processor.
    IntMasterEnable();
    // System delay for accurate initial value calibration
    SysCtlDelay (SysCtlClockGet() / 8);


    landed_height = calibrate_height(); // Set initial helicopter resting height

    while (1)
    {
        // Reset initial helicopter height if left button pushed
        if ((checkButton (LEFT) == PUSHED))
        {
            landed_height = calibrate_height(); // Reset initial helicopter resting height
        }

        //
        // Background task: calculate the (approximate) mean of the values in the
        // circular buffer and display it, together with the sample number.

        sum = 0;

        for (i = 0; i < BUF_SIZE; i++)
            sum = sum + (readCircBuf (&g_inBuffer));
        // Calculate and display the rounded mean of the buffer contents
        mean = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;

        // Set upper limit of ADC value to resting height
        if (mean > landed_height){
            mean = landed_height;
        }

        displayMeanVal (mean, landed_height); // Display helicopter height

        SysCtlDelay (SysCtlClockGet() / 32);  // Update display at ~ 2 Hz
    }
}

