//********************************************************
//
// uartDemo.c - Example code for ENCE361
//
// Link with modules:  buttons2, OrbitOLEDInterface
//
// Author:  P.J. Bones  UCECE
// Last modified:   16.4.2018
//

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"
#include "uart.h"
#include "altitude.h"
#include "yaw.h"

char statusStr[MAX_STR_LEN + 1];

//**********************************************************************
// Initialise UART
//**********************************************************************
void
initUSB_UART (void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    //
    SysCtlPeripheralEnable(UART_USB_PERIPH_UART);
    SysCtlPeripheralEnable(UART_USB_PERIPH_GPIO);
    //
    // Select the alternate (UART) function for these pins.
    //
    GPIOPinTypeUART(UART_USB_GPIO_BASE, UART_USB_GPIO_PINS);
    GPIOPinConfigure (GPIO_PA0_U0RX);
    GPIOPinConfigure (GPIO_PA1_U0TX);

    UARTConfigSetExpClk(UART_USB_BASE, SysCtlClockGet(), BAUD_RATE,
            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
            UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART_USB_BASE);
    UARTEnable(UART_USB_BASE);
}


//**********************************************************************
// Transmit a string via UART0
//**********************************************************************
void
UARTSend (char *pucBuffer)
{
    // Loop while there are more characters to send.
    while(*pucBuffer)
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART_USB_BASE, *pucBuffer);
        pucBuffer++;
    }
}

//**********************************************************************
// Update string to be send via UART
//**********************************************************************
void
UARTTransData (height_data_s height_data, yaw_data_s yaw_data, duty_cycle_s heli_duty, flight_mode current_state, uint8_t slowTick)
{
    char flight_status[10];

    if (slowTick)
    {
        slowTick = false;

        switch (current_state)
        {
        case landed:
            strcpy(flight_status, "Landed");
            break;
        case initialising:
            strcpy(flight_status, "Initialising");
            break;
        case flying:
            strcpy(flight_status, "Take off");
            break;
        case landing:
            strcpy(flight_status, "Landing");
        }

        // Form and send a status message to the console
        usprintf (statusStr, "----------------\r\nAlt: %2d [%2d]\r\nYaw: %2d [%2d]\r\nMain %2d Tail %2d\r\nMode: %s\r\n", height_data.current, height_data.target, yaw_data.current, yaw_data.target, heli_duty.main, heli_duty.tail, flight_status); // * usprintf
        UARTSend (statusStr);
    }

}

