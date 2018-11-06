/*! @file
 *
 *  @brief Routines for setting up functions.
 *
 *  This contains the functions for reading files from a look up table, and manipulating the values bases on the frequency, amplitude, offset, and wave form.
 *  This also contains the struct used to manage each of the two Digital to Anlog converter channels to maintain the values mentions above.
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
#define AWG_NB_CHANNELS 2  		//number of channels being managed
#define AWG_MAX_FREQUENCY 25600		//max frequency that can be processed
#define AWG_MAX_AMPLITUDE 32767		//max amplitude that can be displayed
#define AWG_MIN_OFFSET -32767		//minimum offset that can be displayed
#define ARB_WAVE_SIZE 1000		//max number of samples that can be stored for an arbitrary waveform
#define ARB_HAMONIC_SIZE 5 		//max number of harmonics that can be displayed

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
  uint16_t phase[ARB_HAMONIC_SIZE];
  uint8_t amplitude[ARB_HAMONIC_SIZE];
  uint8_t nbHarmoics;
}TArbitraryHarmonic;

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
  TArbitraryHarmonic harmonicValues;		/*!< Stores the values for the harmonic wavesforms*/
  bool active;					/*!< Indicates if the channel is active or not */
  bool arbitraryHarmonic;			/*!< Indicates if the arbitrary waveform will take in the harmonic values */
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

/*! @brief updates the frequency value for a particular channel.
 *
 *  @param channelNb is the channel who's frequency is to be changed.
 *  @param frequency is the frequency value.
 *  @return bool - true if the channel frequency is changed.
 */
bool AWG_UpdateFrequency(uint8_t channelNb, uint16_t frequency);

/*! @brief updates the amplitude value for a particular channel.
 *
 *  @param channelNb is the channel who's amplitude is to be changed.
 *  @param amplitude is the frequency value.
 *  @return bool - true if the channel amplitude is changed.
 */
bool AWG_UpdateAmplitude(uint8_t channelNb, uint16_t amplitude);

/*! @brief updates the offset value for a particular channel.
 *
 *  @param channelNb is the channel who's offset is to be changed.
 *  @param amplitude is the offset value.
 *  @return bool - true if the channel offset is changed.
 */
bool AWG_UpdateOffset(uint8_t channelNb, uint16_t offset);

/*! @brief updates the amount the index will be jumping for an arbitrary waveform.
 *
 *  @param channelNb is the channel who's amplitude is to be changed.
 *  @return bool - true if the channel index adder is changed.
 */
bool AWG_UpdateArbitraryIndexAdder(uint8_t channelNb);

/*! @brief used to upload the arbitrary waveform samples into ram for any channel.
 *
 *  @param sample is the values to be stored into ram.
 *  @param channelNb is the channel the arbitrary waveform is being uploaded for.
 *  @return bool - true if sample was stored into ram.
 */
bool AWG_UploadAbitraryWave(int16_t sample, uint8_t channelNb);

/*! @brief used to reset the size of an arbitrary waveform array to size to read in new wave
 *
 *  @param channelNb is the channel who's index is being reset.
 */
void AWG_ResetAbitraryWaveSize(uint8_t channelNb);

/*! @brief used to upload the harmonic arbitrary waveform amplitude.
 *
 *  @param channelNb is the channel the arbitrary harmonic waveform is being uploaded for.
 *  @param hamonic is which harmonic to update, 0 being the fundamental and 4 being the fifth
 *  @param amplitude is the actual amplitude of the hamonic
 */
void AWG_UploadHarmonicAmplitude(uint8_t channelNb, uint8_t harmonic, uint16_t amplitude);

/*! @brief used to upload the harmonic arbitrary waveform phase.
 *
 *  @param channelNb is the channel the arbitrary hamonic waveform is being uploaded for.
 *  @param hamonic is which harmonic to update, 0 being the fundamental and 4 being the fifth
 *  @param phase is the actual phase of the hamonic in degrees
 */
void AWG_UploadHarmonicPhase(uint8_t channelNb, uint8_t harmonic, uint16_t phase);

/*! @brief used to update a value to switch between hamonic or csv downloaded arbitrary waveform.
 *
 *  @param channelNb is the channel the arbitrary hamonic waveform is being uploaded for.
 *  @param status turns the status as active or not active
 */
void AWG_UpdateArbitraryStatus(uint8_t channel, uint8_t status);

#endif

/*!
** @}
*/
