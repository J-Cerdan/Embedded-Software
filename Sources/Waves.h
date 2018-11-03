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
**  @addtogroup Waves_module Waves module documentation
**  @{
*/

#ifndef WAVES_H
#define WAVES_H

#include "types.h"

#define MAX_WAVE_SIZE 10000LU

//lookup table generated from https://daycounter.com/Calculators/Sine-Generator-Calculator.phtml
//values between 0 and 32767
extern const uint16_t WAVES_SINEWAVE[MAX_WAVE_SIZE];

extern const uint16_t WAVES_TRIANGLEWAVE[MAX_WAVE_SIZE];

extern const uint16_t WAVES_SQUAREWAVE[MAX_WAVE_SIZE];

extern const uint16_t WAVES_SAWTOOTHWAVE[MAX_WAVE_SIZE];

#endif


/*!
** @}
*/

