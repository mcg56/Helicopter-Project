#ifndef UART_H_
#define UART_H_
// *******************************************************
//
// uart.h
//
// Runs the UART transmission for helicopter data.
// Transmits the state, height and yaw (current and target)
// and the duty cycle values.
//
// Author:  P.J. Bones  UCECE
// Modified by: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified:   19/5/2021
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>
#include "flight_mode.h"
#include "altitude.h"
#include "yaw.h"
#include "responseControl.h"

//********************************************************
// Constants
//********************************************************
#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 40
#define MAX_STR_LEN 100
//---USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE 9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX

//**********************************************************************
// Initialise UART
//**********************************************************************
void
initUSB_UART (void);

//**********************************************************************
// Transmit a string via UART0
//**********************************************************************
void
UARTSend (char *pucBuffer);

//**********************************************************************
// Update string to be send via UART
//**********************************************************************
void
UARTTransData (height_data_s height_data, yaw_data_s yaw_data, duty_cycle_s heli_duty,
               flight_mode current_state, uint8_t slowTick);


#endif /* UART_H_ */
