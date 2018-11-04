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

#include "AWG.h"

typedef struct
{

  uint8_t waveform;      /*!< Waveform Configuration */
  uint16union_t frequency;      /*!< Frequency Configuration */
  uint16union_t amplitude;      /*!< Amplitude Configuration */
  uint16union_t offset;      /*!< Offset Configuration */

} TAWG;

TAWG InputValues;


//Can be used for both offset and amplitude
uint16_t AmplitudeOffsetConversion(uint16_t value)
{
  uint16_t convertedValue;

  convertedValue = (value * 100) / 32767;

  if (((value * 100) % 32767) >= 16384)
  {
    convertedValue++;
  }

  return convertedValue;
}

uint16_t FrequencyConversion(uint16_t frequency)
{
  uint16_t convertedFrequency;

  convertedFrequency = (frequency * 10) / 256;

  if (((frequency * 10) % 256) >= 128)
  {
    convertedFrequency++;
  }

  return convertedFrequency;
}

uint16_t Average(uint16_t index, const uint16_t waveform[])
{
  uint16_t addition, difference, num1, num2;

  num1 = waveform[index/10];

  if (index+10 < 100000)
    {
      num2 = waveform[(index+10)/10];
    }
  else
    {
      num2 = num1;
      num1 = waveform[0];
    }

  difference = num2 - num1;

  addition = difference*(index%10)/10;

  return addition + waveform[index/10];
}



/*!
** @}
*/
