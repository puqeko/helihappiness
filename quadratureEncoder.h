#ifndef QUADRATURE_ENCODER_H_
#define QUADRATURE_ENCODER_H_

//************************************************************************
// quadratureEncoder.h
//
// Helicopter project
//
// Group:       A03 Group 10
// Created:     16/4/18
// Last Edited: 16/4/18
//
// Purpose: Handles input from the quadrature encoder
//************************************************************************

#include <stdint.h>



//************************************************************************
//
//************************************************************************
void quadEncoderIntHandler(void);


//************************************************************************
//
//************************************************************************
void quadEncoderResetCount(void);


//************************************************************************
//
//************************************************************************
int32_t quadEncoderGetCount(void);


//************************************************************************
//
//************************************************************************
void quadEncoderInit(uint32_t, uint32_t, uint32_t, uint32_t);

#endif /*QUADRATURE_ENCODER_H_*/
