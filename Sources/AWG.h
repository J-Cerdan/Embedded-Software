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
#define AWG_MAX_AMPLITUDE 32767
#define AWG_MIN_OFFSET -32767
#define ARB_WAVE_SIZE 1000

typedef enum
{
  SINE_FUNCTION = 0,
  SQUARE_FUNCTION = 1,
  TRIANGLE_FUNCTION = 2,
  SAWTOOTH_FUNCTION = 3,
  NOISE = 4,
  ARBITRARY_FUNCTION = 5
} FunctionWaveForm;

typedef struct
{
  FunctionWaveForm  waveform;			/*!< The wave form for this channel to display. */
  uint16_t frequency;                  		/*!< The frequency of the wave multiplied by 10. */
  uint8_t amplitude;    			/*!< the amplitude of the wave multiplied by 10. */
  int16_t offset;                      		/*!< The offset of the wave when displayed */
  uint16_t index; 				/*!< The index*/
  uint16_t arbitraryWave[ARB_WAVE_SIZE];	/*!< The samples for arbitrary wave form */
  uint16_t sizeOfArbitraryWave;			/*!< number of samples in the arbitrary wave form array */
  uint16_t arbitraryIndexAdder;			/*!< The amount the index must increment for arbitrary wave */
  bool active;					/*!< Indicates if the channel is active or not */
} TAWGDacChannel;

extern TAWGDacChannel AWG_DAC_Channels[AWG_NB_CHANNELS];

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

bool AWG_UpdateOffset(uint8_t channelNb, uint16_t offset);

bool AWG_UpdateArbitraryIndexAdder(uint8_t channelNb);

bool AWG_UploadAbitraryWave(int16_t sample, uint8_t channelNb);

void AWG_ResetAbitraryWave(uint8_t channelNb);

#endif

/*!
** @}
*/
