//************************************************************************
// buttons4.h
//
// Helicopter project
//
// Group:       A03 Group 10
// Last Edited: 20/4/18
// Edited from code by P.J. Bones UCECE
//
// Support for a set of FOUR specific buttons on the Tiva/Orbit.
// The buttons are:  UP and DOWN (on the Orbit daughterboard) plus
// LEFT and RIGHT on the Tiva.
//
// Note that pin PF0 (the pin for the RIGHT pushbutton - SW2 on
//  the Tiva board) needs special treatment - See PhilsNotesOnTiva.rtf.
//************************************************************************

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>
#include <stdbool.h>

// Constants
enum butNames {UP = 0, DOWN, LEFT, RIGHT, SW1, SW2, NUM_BUTS}; //# add SW1 add SW2
enum butStates {RELEASED = 0, PUSHED, NO_CHANGE};

// UP button
#define UP_BUT_PERIPH  SYSCTL_PERIPH_GPIOE
#define UP_BUT_PORT_BASE  GPIO_PORTE_BASE
#define UP_BUT_PIN  GPIO_PIN_0
#define UP_BUT_NORMAL  false

// DOWN button
#define DOWN_BUT_PERIPH  SYSCTL_PERIPH_GPIOD
#define DOWN_BUT_PORT_BASE  GPIO_PORTD_BASE
#define DOWN_BUT_PIN  GPIO_PIN_2
#define DOWN_BUT_NORMAL  false

// LEFT button
#define LEFT_BUT_PERIPH  SYSCTL_PERIPH_GPIOF
#define LEFT_BUT_PORT_BASE  GPIO_PORTF_BASE
#define LEFT_BUT_PIN  GPIO_PIN_4
#define LEFT_BUT_NORMAL  true

// RIGHT button
#define RIGHT_BUT_PERIPH  SYSCTL_PERIPH_GPIOF
#define RIGHT_BUT_PORT_BASE  GPIO_PORTF_BASE
#define RIGHT_BUT_PIN  GPIO_PIN_0
#define RIGHT_BUT_NORMAL  true

// SW1 slider switch
#define SW1_PERIPH  SYSCTL_PERIPH_GPIOA
#define SW1_PORT_BASE  GPIO_PORTA_BASE
#define SW1_PIN  GPIO_PIN_7
#define SW1_NORMAL  false

// SW2 slider switch
#define SW2_PERIPH  SYSCTL_PERIPH_GPIOA
#define SW2_PORT_BASE  GPIO_PORTA_BASE
#define SW2_PIN GPIO_PIN_6
#define SW2_NORMAL true

#define NUM_BUT_POLLS 3
// Debounce algorithm: A state machine is associated with each button.
// A state change occurs only after NUM_BUT_POLLS consecutive polls have
// read the pin in the opposite condition, before the state changes and
// a flag is set.  Set NUM_BUT_POLLS according to the polling rate.


// buttonsInit: Initialise the variables associated with the set of buttons
// defined by the constants above.
void buttonsInit (void);


// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
void buttonsUpdate (void);


// checkButton: Function returns the new button state if the button state
// (PUSHED or RELEASED) has changed since the last call, otherwise returns
// NO_CHANGE.  The argument butName should be one of constants in the
// enumeration butStates, excluding 'NUM_BUTS'. Safe under interrupt.
uint8_t buttonsCheck (uint8_t butName);


// ignore a change in state which may have occured for this button.
void buttonsIgnore(uint8_t butName);

#endif /*BUTTONS_H_*/
