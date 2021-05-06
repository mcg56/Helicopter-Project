//*****************************************************************************
//
// altitude.c - helicopter altitute functionality
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
#include "driverlib/pwm.h" // For setting pwm output true
#include "utils/ustdlib.h"
#include "altitude.h"
#include "circBufT.h"
#include "pwmGen.h"
#include "responseControl.h"
#include "driverlib/timer.h"

//*****************************************************************************
// Global Variables
//*****************************************************************************
static circBuf_t g_inBuffer;
static int16_t height_percent;
static int16_t target_height_percent;
static volatile int16_t pwm_main_duty;

//*****************************************************************************
// The interrupt handler for the for Timer interrupt.
//*****************************************************************************
void
AltitudeControlIntHandler (void)
{
    // Clear the timer interrupt flag
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    pwm_main_duty = dutyResponseMain(height_percent, target_height_percent);
    Acount++;
    setPWMMain (PWM_MAIN_FREQ, pwm_main_duty);

}

//*****************************************************************************
// Intialise timer for PI control update
//*****************************************************************************
void
initAltTimer (void)
{
    //
    // The Timer0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //
    // Configure Timer0B as a 16-bit periodic timer.
    //
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000);

    TimerIntRegister(TIMER0_BASE, TIMER_A, AltitudeControlIntHandler);

    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
// Initialise altitude module
//*****************************************************************************
void
initAltitude(void)
{
    SysCtlPeripheralReset(SYSCTL_PERIPH_TIMER0); // REMOVE

    initCircBuf (&g_inBuffer, BUF_SIZE);
    initADC ();
    initialisePWMMain ();
    initAltTimer ();

    // Initialisation is complete, so turn on the output.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);
}

//*****************************************************************************
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
// Sourced from:  P.J. Bones  UCECE
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
// Initialise ADC functions
// Sourced from:  P.J. Bones  UCECE
//*****************************************************************************
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
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE |
                             ADC_CTL_END); // Channel 9 (PE4)

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

//*****************************************************************************
// Function to convert helicopter height to percentage
//*****************************************************************************
int
calculate_percent_height(uint16_t current_height, uint16_t landed_height)
{
    int16_t height_percent;
    float height_voltage_range = 0.8; // Voltage range over helicopter height
    float ADC_voltage_range = 3.33;   // Total voltage range of ADC

    // Percentage height calculation
    height_percent = 100 * ((landed_height - current_height)/((height_voltage_range/ADC_voltage_range)*ADC_BITS));

    return height_percent;
}

//*****************************************************************************
// Find and return current helicopter landed height.
//*****************************************************************************
int
getHeight(void)
{
    int32_t sum;
    int32_t mean;
    uint16_t i;
    //
    // Background task: calculate the (approximate) mean of the values in the
    // circular buffer and display it, together with the sample number.
    sum = 0;
    for (i = 0; i < BUF_SIZE; i++)
        sum = sum + (readCircBuf (&g_inBuffer));

    // Calculate the rounded mean of the buffer contents
    mean = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;

    return mean;
}

//*****************************************************************************
// Update helicopter altitute control
//*****************************************************************************
int16_t
updateAltitude(int16_t height_percent_in, int16_t target_height_percent_in)
{
    height_percent = height_percent_in;
    target_height_percent = target_height_percent_in;

    return pwm_main_duty;
}
