//*****************************************************************************
//
// yaw.c - yaw functionality
//
// Authors: T.R. Peterson M.G. Gardyne M. Comber
// Date 19/04/2021
//
// Code Sourced from:  P.J. Bones  UCECE (acknowledged in function descriptions)

#include <stdint.h>
#include <stdbool.h>
#include "stdlib.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "yaw.h"
#include "responseControl.h"
#include "pwmGen.h"
#include "driverlib/pwm.h" // For setting pwm output true

//*****************************************************************************
// Global variables
//*****************************************************************************
static bool a_cur;                  // Current A-phase pin value
static bool b_cur;                  // Current B-phase pin value
static bool ref_found;              // Helicopter heading in degrees
static bool ref_enabled = false;
static int32_t yaw;             // Helicopter heading from quadrature code disc
static int32_t deg;                 // Helicopter heading in degrees
static int32_t yaw_sweep_duty = 50;


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
    calculateYaw(a_next, b_next);

    // Update next phase values to current
    a_cur = a_next;
    b_cur = b_next;

    // Clean up, clearing the interrupt
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

//*************************************************************
// GPIO Pin Interrupt
//*************************************************************
void
GPIORefPinIntHandler (void)
{
    if (ref_enabled) {
        yaw = 0;
        deg = 0;
        ref_found = true;
    }
    // Clean up, clearing the interrupt
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);
}

//*************************************************************
// Intialise GPIO Pins
//*************************************************************
void
initGPIOPins (void)
{
    // Enable port peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    // Set pin 0,1 and 4 as input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);

    // Set what pin interrupt conditions
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_BOTH_EDGES);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_RISING_EDGE);

    // Register interrupt
    GPIOIntRegister(GPIO_PORTB_BASE, GPIOPinIntHandler);
    GPIOIntRegister(GPIO_PORTC_BASE, GPIORefPinIntHandler);

    // Enable pins
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 );
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_4 );



}

//*************************************************************
// Intialise Yaw module
//*************************************************************
void
initYaw (void)
{
    initGPIOPins ();
    initialisePWMTail ();

    // Initialisation is complete, so turn on the output.
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);
}


//*****************************************************************************
// Function to update helicopter yaw in degrees
//*****************************************************************************
void
calculateYaw(bool a_next, bool b_next)
{
    bool cw;
    uint16_t full_rot = 360;
    uint16_t tooth_count = 448; // Total teeth in quadrature code disc


    // Find rotation direction using current and next phase values
    cw = (!a_cur & !b_cur & !a_next & b_next) | (!a_cur & b_cur & a_next & b_next)
            | (a_cur & b_cur & a_next & !b_next) | (a_cur & !b_cur & !a_next & !b_next);

    // Update yaw based on rotation direction
    if (cw) {
        yaw++;
    } else {
        yaw--;
    }

    /*
    // Limit yaw values
    if (yaw == tooth_count) {
        yaw = 0;
    } else if (yaw == -1) {
        yaw = tooth_count - 1;
    }*/

    // Convert yaw value to degrees with rounded value
    deg = (2 * yaw * full_rot + 1)/(2 * tooth_count);
}

//*****************************************************************************
// Update yaw helicopter control
//*****************************************************************************
uint32_t
updateYaw(int16_t yaw_degree, int16_t target_yaw)
{
    uint32_t pwm_tail_duty;

    pwm_tail_duty = dutyResponseTail(yaw_degree, target_yaw);

    setPWMTail (PWM_MAIN_FREQ, pwm_tail_duty);
    return pwm_tail_duty;
}

void
findReference(void)
{
    ref_enabled = true;
    // Do a sweep
    // If pin someting high set yaw = 0
    while (ref_found == false) {
        setPWMTail (PWM_MAIN_FREQ, yaw_sweep_duty);
    }

    ref_enabled = false;
}

//*****************************************************************************
// Pass deg to main function
//*****************************************************************************
int
getYaw(void)
{
    return deg;
}

