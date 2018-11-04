/*! @file
 *
 *  @brief Used to obtain waveform values
 *
 *  This contains the values of the different waveforms
 *
 *  @author Joseph Cerdan
 *  @date 2018-11-03
 */
/*!
**  @addtogroup Waveform_module Waveform module documentation
**  @{
*/

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "types.h"

#define MAX_WAVE_SIZE 10000LU

//lookup table generated from https://daycounter.com/Calculators
//values between 0 and 65,535
extern const uint16_t WAVEFORM_SINEWAVE[MAX_WAVE_SIZE];

extern const uint16_t WAVEFORM_TRIANGLEWAVE[MAX_WAVE_SIZE];

extern const uint16_t WAVEFORM_SQUAREWAVE[MAX_WAVE_SIZE];

extern const uint16_t WAVEFORM_SAWTOOTHWAVE[MAX_WAVE_SIZE];

#endif


/*!
** @}
*/

