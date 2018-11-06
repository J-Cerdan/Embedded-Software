/*! @file
 *
 *  @brief look up tables for various functions.
 *
 *  This contains the look up tables for values to store values for sine, sawtooth, triangle, and square waves. These values are stored in flash as they are const.
 *
 *  @author Amir Hussein
 *  @date 2018-11-4
 */
/*!
**  @addtogroup functions_module functions module documentation
**  @{
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "types.h"

#define MAX_WAVE_SIZE 10000LU	//max size of the loop up tables

//lookup table containing the values for the various waves
extern const uint16_t FUNCTIONS_SINEWAVE[MAX_WAVE_SIZE];

extern const uint16_t FUNCTIONS_TRIANGLEWAVE[MAX_WAVE_SIZE];

extern const uint16_t FUNCTIONS_SQUAREWAVE[MAX_WAVE_SIZE];

extern const uint16_t FUNCTIONS_SAWTOOTHWAVE[MAX_WAVE_SIZE];

#endif


/*!
** @}
*/
