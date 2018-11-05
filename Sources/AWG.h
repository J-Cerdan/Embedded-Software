/*! @file
 *
 *  @brief Routines for setting up functions.
 *
 *  This contains the functions for reading files from a look up table, and manipulating the values bases on the frequency, amplitude, offset, and wave form.
 *  This also contains the struct used to manage each of the two Digital to Anlog convert channels to maintain the values mentions above.
 *  THis also contains functions to store and manage the arbitrary waves sent from a pc.
 *
 *  @author Amir Hussein
 *  @date 2018-11-4
 */
/*!
**  @addtogroup AWG_module AWG module documentation
**  @{
*/

#ifndef AWG_H
#define AWG_H

// new types
#include "types.h"

// Maximum number of channels
#define AWG_NB_CHANNELS 2
#define AWG_MAX_FREQUENCY 25600

typedef enum
{
  SINE_FUNCTION = 0,
  SQUARE_FUNCTION = 1,
  SAWTOOTH_FUNCTION = 2,
  TRIANGLE_FUNCTION = 3
} FunctionWaveForm;

typedef struct
{
  FunctionWaveForm  waveform;		/*!< The wave form for this channel to display. */
  uint16_t frequency;                  	/*!< The frequency of the wave multiplied by 10. */
  uint8_t amplitude;    		/*!< the amplitude of the wave multiplied by 10. */
  uint8_t offset;                       /*!< The offset of the wave when displayed */
  uint16_t index; /*!< The index */
  bool active;
} TAWGDacChannel;

extern TAWGDacChannel AWG_DAC_CHANNELS[AWG_NB_CHANNELS];

/*! @brief Sets up the DAC Channels before first use.
 *
 *  @return bool - true if the Channels were was successfully initialized.
 */
bool AWG_Init(void);

/*! @brief returns a sample value for a function.
 *
 *  @param channelNb is the number of the analog output channel to send sample.
 *  @return uint16_t - the value of the sample.
 */
uint16_t AWG_SampleGet(uint8_t channelNb);


bool AWG_UpdateFrequency(uint8_t channelNb, uint16_t frequency);


bool AWG_UpdateAmplitude(uint8_t channelNb, uint16_t amplitude);


#endif

/*!
** @}
*/
