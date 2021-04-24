//*****************************************************************************
//
// yaw.c - yaw functionality
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
static int32_t deg;                 // Helicopter heading in degrees
static bool a_cur;                  // Current A-phase pin value
static bool b_cur;                  // Current B-phase pin value

void updateYaw(bool a_next, bool b_next);

//*************************************************************
// GPIO Pin Interrupt
//*************************************************************
void
GPIOPinIntHandler (void)
{
    bool a_next;
    bool b_next;

    // Read next A-phase and B-phase values
    a_next = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_0);
    b_next = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1);

    // Update yaw in degrees
    updateYaw(a_next, b_next);

    // Update next phase values to current
    a_cur = a_next;
    b_cur = b_next;

    // Clean up, clearing the interrupt
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

//*************************************************************
// Intialise GPIO Pins
//*************************************************************
void
initGPIOPins (void)
{
    // Enable port peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Set pin 0 and 1 as input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Set what pin interrupt conditions
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_BOTH_EDGES);

    // Register interrupt
    GPIOIntRegister(GPIO_PORTB_BASE, GPIOPinIntHandler);

    // Enable pins
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 );

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
    uint16_t scale_correction = 10000; // Scale multiplier for float calculations
    uint16_t tooth_count = 440; // Total teeth in quadrature code disc


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
    if (yaw == tooth_count) {
        yaw = 0;
    } else if (yaw == -1) {
        yaw = tooth_count - 1;
    }

    // Convert yaw value to degrees
    deg_f = yaw * scale_factor * scale_correction;
    deg = deg_f / scale_correction;
}
