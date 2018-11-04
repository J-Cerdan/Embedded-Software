#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "types.h"

#define MAX_WAVE_SIZE 10000LU

//lookup table contianting the values for the various waves
//values between 0 and 32767
extern const uint16_t FUNCTIONS_SINEWAVE[MAX_WAVE_SIZE];

extern const uint16_t FUNCTIONS_TRIANGLEWAVE[MAX_WAVE_SIZE];

extern const uint16_t FUNCTIONS_SQUAREWAVE[MAX_WAVE_SIZE];

extern const uint16_t FUNCTIONS_SAWTOOTHWAVE[MAX_WAVE_SIZE];

#endif
