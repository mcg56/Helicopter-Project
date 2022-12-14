#ifndef PWMGEN_H_
#define PWMGEN_H_

// *******************************************************
// pwmGen.h
//
// Generates PWM signals for helicopter height
// and yaw control
//
// Authors: T.R. Peterson, M.G. Gardyne, M. Comber
// Last modified 19/5/2021
//
// Original code Sourced from:  P.J. Bones  UCECE, and modified by authors
//
// *******************************************************

#include <stdint.h>
#include <stdio.h>

/**********************************************************
 * Constants
 **********************************************************/

// PWM configuration
#define PWM_DIVIDER_CODE   SYSCTL_PWMDIV_1
#define PWM_DIVIDER        1
#define PWM_MAIN_FREQ      250
#define PWM_START_DUTY     0
#define PWM_TAIL_FREQ      250

//  PWM Hardware Details M0PWM7 (gen 3)
//  ---Main Rotor PWM: PC5, J4-05
#define PWM_MAIN_BASE        PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM  SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5


//  PWM Hardware Details M1PWM5 (gen 2)
//  ---Tail Rotor PWM: PF1, J3-10
#define PWM_TAIL_BASE        PWM1_BASE
#define PWM_TAIL_GEN         PWM_GEN_2
#define PWM_TAIL_OUTNUM      PWM_OUT_5
#define PWM_TAIL_OUTBIT      PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM  SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE   GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN    GPIO_PIN_1

/*********************************************************
 * initialisePWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 *********************************************************/
void
initialisePWMMain (void);

/*********************************************************
 * initialisePWM
 * M1PWM5 (J3-10, PF1) is used for the tail rotor motor
 *********************************************************/
void
initialisePWMTail (void);

/********************************************************
 * Function to set the freq, duty cycle of M1PWM5
 ********************************************************/
void
setPWMTail (uint32_t ui32Freq, float ui32Duty);

/********************************************************
 * Function to set the freq, duty cycle of M0PWM7
 ********************************************************/
void
setPWMMain (uint32_t ui32Freq, float ui32Duty);

#endif /* PWMGEN_H_ */
