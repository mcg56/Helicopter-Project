#ifndef UART_H_
#define UART_H_
// *******************************************************
//
// switches.h
//
// Support for the up button Tiva/Orbit.
// ENCE361 sample code.
//
//
// Note that pin PF0 (the pin for the RIGHT pushbutton - SW2 on
//  the Tiva board) needs special treatment - See PhilsNotesOnTiva.rtf.
//
// TRP
// Last modified:  26.04.2020
//
// *******************************************************

#include <stdint.h>
#include <stdbool.h>


void
initialiseUSB_UART (void);

//**********************************************************************
// Transmit a string via UART0
//**********************************************************************
void
UARTSend (char *pucBuffer);



#endif /* UART_H_ */
