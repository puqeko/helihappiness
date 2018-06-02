//************************************************************************
// quadratureEncoder.h
// Helicopter project
// Group: A03 Group 10
// Last Edited: 31-5-18
//
// Purpose: Handles input from the quadrature encoder
//************************************************************************

#ifndef QUADRATURE_ENCODER_H_
#define QUADRATURE_ENCODER_H_

#include <stdint.h>
#include <stdbool.h>


// Resets the running encoder count to zero
void quadEncoderResetCount(void);


// Set the running encoder count to the parameter newCount
//
// WARNING: When setting count value based on the existing count value,
//          do so in an atomic manner as the existing count value could
//          change before it is set to the new count value. Failing to
//          do so could result in the zero position "moving" over time.
void quadEncoderSetCount(uint32_t newCount);


// Returns the current encoder count
//
// WARNING: Function must be used in an atomic manner as the encoder count
//          can change at any time because it is interrupt driven
int32_t quadEncoderGetCount(void);


// Configure GPIO pins and initialize interrupts
// Get initial state of channel pins for determining direction on first interrupt
void quadEncoderInit(void);

#endif /*QUADRATURE_ENCODER_H_*/
