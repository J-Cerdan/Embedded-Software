/*! @file
 *
 *  @brief AWG Configurations and functions
 *
 *  This contains AWG configurations and functions
 *
 *  @author Joseph Cerdan
 *  @date 2018-11-03
 */
/*!
**  @addtogroup AWG_module AWG module documentation
**  @{
*/

#include "types.h"

#ifndef SOURCES_AWG_H_
#define SOURCES_AWG_H_

#define NB_DAC_CHANNELS 2


typedef enum
{
  WAVEFORMS_SINEWAVE = 0,
  WAVEFORMS_SQUAREWAVE = 1,
  WAVEFORMS_TRIANGLEWAVE = 2,
  WAVEFORMS_SAWTOOTHWAVE = 3
} TWaveforms;

typedef struct
{
  bool active;		/*!< Active Configuration */
  TWaveforms waveform;     /*!< Waveform Configuration */
  uint16_t frequency;    /*!< Frequency Configuration */
  uint8_t amplitude;    /*!< Amplitude Configuration */
  uint16_t offset;      /*!< Offset Configuration */
  uint16_t index;	/*!< Offset Configuration */

} TAWGChannel;

extern TAWGChannel DACChannel[NB_DAC_CHANNELS];


/*! @brief Function to convert amplitude/offset value needed for waveform
 *
 *  @param value is the amplitude/offset received to be converted
 *
 *  @return returns the amplitude/offset in correct form
 *
 */
uint16_t AmplitudeOffsetConversion(uint16_t value);

/*! @brief Function to convert frequency value needed for waveform
 *
 *  @param frequency - is the frequency received to be converted
 *
 *  @return returns the frequency in correct form
 *
 */
uint16_t FrequencyConversion(uint16_t frequency);


/*! @brief Function to calculate value needed for waveform
 *
 *  @param index is the index position of the value required for the function
 *  @param waveform is the waveform array required to obtain the value
 *
 *  @return returns the averaged value of the two values
 *
 */
uint16_t Average(uint16_t index, const uint16_t waveform[]);



#endif /* SOURCES_AWG_H_ */

/*!
** @}
*/
