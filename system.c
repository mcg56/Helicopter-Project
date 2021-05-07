//*****************************************************************************
//
// system.c - helicopter system functionality
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
#include "utils/ustdlib.h"
#include "buttons4.h"
#include "yaw.h"
#include "altitude.h"
#include "display.h"
#include "circBufT.h"
#include "pwmGen.h"
#include "uart.h"

static volatile uint8_t slowTick = false;

//*****************************************************************************
// The interrupt handler for the for Clock interrupt.
//*****************************************************************************
void
SysTickIntHandler(void)
{
    static uint8_t tickCount = 0;
    const uint8_t ticksPerSlow = SYSTICK_RATE_HZ / SLOWTICK_RATE_HZ;

    // Poll the buttons
    updateButtons();

    ADCProcessorTrigger(ADC0_BASE, 3);

    // Update slowTick value for UART transmission
    if (++tickCount >= ticksPerSlow)
    {
        tickCount = 0;
        slowTick = true;
    }
}

//*****************************************************************************
// The interrupt handler for the for soft reset interrupt.
//*****************************************************************************
void
SoftResetIntHandler (void)
{
    SysCtlReset();

    // Clean up, clearing the interrupt
    GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_6);
}

//*****************************************************************************
// Initialisation function for the clock
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

//*****************************************************************************
// Initialisation function for soft reset pin
//*****************************************************************************
void
initSoftReset (void)
{
    // Enable port peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Set pin 0 and 1 as input
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6);

    // Set pad configuration
    GPIOPadConfigSet (GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA,
           GPIO_PIN_TYPE_STD_WPU);

    // Register interrupt
    GPIOIntRegister(GPIO_PORTA_BASE, SoftResetIntHandler);

    // Enable pin
    GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_6);
}

//*************************************************************
// Pass slowTick to main for UART transmission
//*************************************************************
uint8_t
getSlowTick(void)
{
   return slowTick;
}
