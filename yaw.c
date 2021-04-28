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

//*****************************************************************************
// Global variables
//*****************************************************************************
static bool a_cur;                  // Current A-phase pin value
static bool b_cur;                  // Current B-phase pin value
static int32_t yaw = 0;             // Helicopter heading from quadrature code disc
static int32_t deg;                 // Helicopter heading in degrees

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


    // Convert yaw value to degrees
    deg = yaw * full_rot/tooth_count;
}

//*****************************************************************************
// Update yaw helicopter control
//*****************************************************************************
void
updateYaw(int16_t yaw_degree, int16_t target_yaw)
{
    uint32_t pwm_tail_duty;

    pwm_tail_duty = dutyResponse(yaw_degree, target_yaw);

    setPWMMain (PWM_MAIN_FREQ, pwm_tail_duty);
}

//*****************************************************************************
// Pass deg to main function
//*****************************************************************************
int
getYaw(void)
{
    return deg;
}

