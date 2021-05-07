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
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h" // For setting pwm output true
#include "yaw.h"
#include "responseControl.h"
#include "pwmGen.h"

//#include "driverlib/interrupt.h"
//#include "driverlib/debug.h"
//#include "inc/hw_ints.h"
//#include "stdlib.h"

//*****************************************************************************
// Global variables
//*****************************************************************************
static bool a_cur;                  // Current A-phase pin value
static bool b_cur;                  // Current B-phase pin value
static bool ref_found;              // Helicopter heading in degrees
static volatile bool ref_enabled = false;

static int16_t yaw;                 // Helicopter heading from quadrature code disc
static int16_t yaw_degree;          // Helicopter heading in degrees
static int16_t target_yaw;

static uint32_t pwm_tail_duty;


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
// GPIO reference yaw pin interrupt
//*************************************************************
void
GPIORefPinIntHandler (void)
{
    // Set reference found to true and reset yaw values
    if (ref_enabled) {
        yaw = 0;
        yaw_degree = 0;
        ref_found = true;
    }

    ref_enabled = false;

    // Clean up, clearing the interrupt
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);
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


//    // Limit yaw values
//    if (yaw == tooth_count) {
//        yaw = 0;
//    } else if (yaw == -1) {
//        yaw = tooth_count - 1;
//    }



//    // Limit yaw values
//    if ((yaw == (tooth_count/2 + 1))) {
//        yaw = -1 * tooth_count/2 - 1;
//    } else if ((yaw == -1 * tooth_count/2)) {
//        yaw = tooth_count/2;
//    }


    // Convert yaw value to degrees with rounded value
    yaw_degree = (2 * yaw * full_rot + 1)/(2 * tooth_count);
}

//*****************************************************************************
// Update yaw helicopter control
//*****************************************************************************
uint32_t
updateYaw(int16_t yaw_degree_in, int16_t target_yaw_in)
{
    yaw_degree = yaw_degree_in;
    target_yaw = target_yaw_in;
    pwm_tail_duty = getTailDuty();

    return pwm_tail_duty;
}

//*****************************************************************************
// Sweep helicopter to find reference yaw
//*****************************************************************************
bool
findReference(void)
{
    ref_enabled = true;
    var_check++;

    // Alternative method
    //    ref_enabled = true;
    //
    //    // Sweep helicopter at fixed rate
    //    while (ref_found == false) {
    //        setPWMTail (PWM_MAIN_FREQ, yaw_sweep_duty);
    //        setPWMMain (PWM_TAIL_FREQ, height_sweep_duty);
    //
    //    }
    //
    //    ref_enabled = false;

    return ref_found;

}

//*****************************************************************************
// Pass current yaw to other modules
//*****************************************************************************
int16_t
getYaw(void)
{
    return yaw_degree;
}

//*****************************************************************************
// Pass yaw target to reponse control module
//*****************************************************************************
int16_t
getYawTarget(void)
{
    return target_yaw;
}

//*****************************************************************************
// Pass reference found to reponse control module
//*****************************************************************************
bool
refFound(void)
{
    return ref_found;
}
