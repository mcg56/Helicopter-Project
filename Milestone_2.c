//*****************************************************************************
//
// Milestone_1.c - Simple voltage measure of helicopter altitude
//
// Authors: Tom Peterson, Matt Comber, Mark Gardyne
// Date 8/03/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)
//


#include <stdint.h>
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
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"


//*****************************************************************************
// Constants
//*****************************************************************************
#define BUF_SIZE            10   // Buffer size for sample averaging
#define SAMPLE_RATE_HZ      2000 // Sample rate
#define SYSTICK_RATE_HZ     100  // Systick configuration
#define ADC_BITS            4095 // 12 bit ADC

//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;        // Counter for the interrupts
static int32_t yaw;                 // Helicopter heading from quadrature code disc
static int32_t deg;                // Helicopter heading in degrees
static bool a_cur;                  // Current A-phase pin value
static bool b_cur;                  // Current B-phase pin value

//*****************************************************************************
// Function handles
//*****************************************************************************
void updateYaw(bool a_next, bool b_next);

//*****************************************************************************
// The interrupt handler for the for SysTick interrupt.
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
}

//*****************************************************************************
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
// Code Sourced from:  P.J. Bones  UCECE
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

//*************************************************************
// GPIO Pin Interrupt
//*************************************************************
void
GPIOPinIntHandler (void)
{
    bool a_next;
    bool b_next;

    // Set next A-phase and B-phase values
    a_next = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0);
    b_next = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1);

    // Update yaw in degrees
    updateYaw(a_next, b_next);

    // Set next phase values to current
    a_cur = a_next;
    b_cur = b_next;

    // Clear pin interrupts
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

//*****************************************************************************
// Initialisation functions for the clock (incl. SysTick), ADC, display
// Code Sourced from:  P.J. Bones  UCECE
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

//*****************************************************************************
// Initialise ADC functions
// Code Sourced from:  P.J. Bones  UCECE
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

void
initDisplay (void)
{
    // intialise the Orbit OLED display
    OLEDInitialise ();
}

//*************************************************************
// SysTick interrupt
// Code Sourced from:  P.J. Bones  UCECE
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

//*************************************************************
// Intialise GPIO Port B pins 0 and 1
//*************************************************************
void
initGPIOPins (void)
{
    // Enable port peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Set pin 0 and 1 as input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Set what conditions pins will be interrupted on
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_BOTH_EDGES);

    // Register interrupt
    GPIOIntRegister(GPIO_PORTB_BASE, GPIOPinIntHandler);

    // Enable pins
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 );

}

//*****************************************************************************
// Function to calculate the helicopter height as a percent of the voltage range
//*****************************************************************************
int
calculate_percent_height(uint16_t current_height, uint16_t landed_height)
{
    int16_t height_percent;
    float height_voltage_range = 0.8; // Voltage range over helicopter height
    float ADC_voltage_range = 3.33;   // Total voltage range of ADC

    // Calculate helicopter height as percentage
    height_percent = 100 * ((landed_height - current_height)/((height_voltage_range/ADC_voltage_range)*ADC_BITS));

    return height_percent;
}

//*****************************************************************************
// Function to display either the mean ADC value, the helicopter height as a percentage
// or no display at all.
//*****************************************************************************
void
displayMeanVal(uint16_t meanVal, uint16_t landed_height, int8_t display_state)
{
    char string[17];  // 16 characters across the display
    int16_t height_percent;

    switch (display_state)
    {
    case 0:
        height_percent = calculate_percent_height(meanVal, landed_height);
        usnprintf (string, sizeof(string), "Height   %5d%%", height_percent);
        OLEDStringDraw (string, 0, 0);
        usnprintf (string, sizeof(string), "Yaw (deg) %5d", deg);
        OLEDStringDraw (string, 0, 1);
        break;
    case 1:
        usnprintf (string, sizeof(string), "Mean ADC = %4d", meanVal);
        OLEDStringDraw (string, 0, 0);
        break;
    case 2:
        OLEDStringDraw ("                ", 0, 0);
        OLEDStringDraw ("                ", 0, 1);
    }
}

//*****************************************************************************
// Function to find the current voltage reading and record this as the landed helicopter height.
//*****************************************************************************
int
calibrate_height()
{
    int32_t start_height;
    int32_t sum;
    uint32_t i;

    sum = 0;
    for (i = 0; i < BUF_SIZE; i++)
        sum = sum + (readCircBuf (&g_inBuffer));
    // Calculate and display the rounded mean of the buffer contents
    start_height = ((2 * sum + BUF_SIZE) / 2 / BUF_SIZE);

   return start_height;
}

//*****************************************************************************
// Function to update helicopter yaw in degrees
//*****************************************************************************
void
updateYaw(bool a_next, bool b_next)
{
    bool cw;
    float deg_f;
    float scale_factor = 0.8181; // Scale factor for converting yaw to degrees (360/440 to 4dp)


    // Find rotation direction using current and next phase values
    cw = (!a_cur & !b_cur & !a_next & b_next) | (!a_cur & b_cur & a_next & b_next)
            | (a_cur & b_cur & a_next & !b_next) | (a_cur & !b_cur & !a_next & !b_next);

    // Update yaw based on rotation direction
    if (cw) {
        yaw++;
    } else {
        yaw--;
    }

    // Limit yaw values
    if (yaw == 440) {
        yaw = 0;
    } else if (yaw == -1) {
        yaw = 439;
    }

    // Convert yaw value to degrees
    deg_f = yaw * scale_factor * 10000;
    deg = deg_f / 10000;
    // FIX THIS!!!
}


int
main(void)
{
    uint16_t i;
    int32_t sum;
    int32_t mean;
    int32_t landed_height;
    int8_t display_state;

    SysCtlPeripheralReset (LEFT_BUT_PERIPH);    // LEFT button GPIO
    SysCtlPeripheralReset (UP_BUT_PERIPH);      // UP button GPIO
    SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB); // Port B pins

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
    display_state = 0;                  // Set initial display state to percentage
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
            display_state++;
            if (display_state == 3) {
                display_state = 0;
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
