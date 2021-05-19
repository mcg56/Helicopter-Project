//*****************************************************************************
//
// yaw.c
//
// Helicopter yaw functionality. Reads and updates yaw via interrupts to GPIO
// pins reading from a quadrature encoder disk. Yaw range is 180 to -179.
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified: 19/5/2021
//

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "yaw.h"
#include "responseControl.h"
#include "pwmGen.h"

//*****************************************************************************
// Global variables
//*****************************************************************************
static bool a_cur;                          // Current A-phase pin value
static bool b_cur;                          // Current B-phase pin value
static bool ref_found;                      // Reference yaw found flag
static volatile bool ref_enabled = false;   // Enable reference yaw pin interrupt

static yaw_data_s yaw_data;                 // Yaw current and target values
static int16_t yaw;                         // Helicopter heading from quadrature code disc

//*****************************************************************************
// Function prototypes
//*****************************************************************************
void calculateYaw(bool a_next, bool b_next);

//*************************************************************
// GPIO Pin Interrupt
//*************************************************************
void
GPIOPinIntHandler (void)
{
    // Clean up, clearing the interrupt
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

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
}

//*************************************************************
// GPIO reference yaw pin interrupt
//*************************************************************
void
GPIORefPinIntHandler (void)
{
    // Clean up, clearing the interrupt
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);

    // Set ref_found to true and reset yaw values if interrupt enabled
    if (ref_enabled) {
        yaw = 0;
        yaw_data.current = 0;
        ref_found = true;
    }

    ref_enabled = false;
}

//*************************************************************
// Intialise GPIO Pins
// PB0 and PB1 are used for quadrature encoding
// PC4 is used for reference yaw input
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
    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (PWM_TAIL_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_TAIL_PERIPH_PWM);  // Tail Rotor PWM

    initGPIOPins ();
    initialisePWMTail ();
    initResponseTimer ();

    // Initialisation is complete, so turn on the output.
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);
}


//*****************************************************************************
// Update helicopter yaw in degrees
//*****************************************************************************
void
calculateYaw(bool a_next, bool b_next)
{
    bool cw;
    int16_t full_rot = 360;    // Degrees in full rotation
    int16_t tooth_count = 448; // Total count in quadrature code disc

    // Find rotation direction using current and next phase values
    cw = (!a_cur & !b_cur & !a_next & b_next) | (!a_cur & b_cur & a_next & b_next)
            | (a_cur & b_cur & a_next & !b_next) | (a_cur & !b_cur & !a_next & !b_next);

    // Update yaw based on rotation direction
    if (cw) {
        yaw++;
    } else {
        yaw--;
    }

    // Limit yaw values for 180 to -179 degree range
    if (yaw >= tooth_count/2 && cw) {
        yaw = -1 * tooth_count/2 + 1;
    } else if (yaw <= -1 * tooth_count/2 && !cw) {
        yaw = tooth_count/2;
    }

    // Convert yaw value to degrees with rounded value
    yaw_data.current = (2 * yaw * full_rot + 1)/(2 * tooth_count);
}

//*****************************************************************************
// Sweep helicopter to find reference yaw
//*****************************************************************************
bool
findReference(void)
{
    // Enable reference yaw pin interrupt
    ref_enabled = true;

    return ref_found;
}

//*****************************************************************************
// Pass current yaw out of module
//*****************************************************************************
int16_t
getYawCurrent(void)
{
    return yaw_data.current;
}

//*****************************************************************************
// Pass reference found out of module
//*****************************************************************************
bool
refFound(void)
{
    return ref_found;
}
