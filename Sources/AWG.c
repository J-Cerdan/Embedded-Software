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
#include "PE_Types.h"
#include "Waveform.h"

TAWGChannel DACChannel[NB_DAC_CHANNELS];

bool AWG_Init()
{

  for (uint8_t i=0; i < NB_DAC_CHANNELS; i++)
  {
    DACChannel[i].active = FALSE;
    DACChannel[i].waveform = WAVEFORMS_SINEWAVE;
    DACChannel[i].frequency = 10;
    DACChannel[i].amplitude = 10;
    DACChannel[i].offset = 0;
    DACChannel[i].index = 0;
  }

  return TRUE;

}

uint16_t VoltageAdjust(uint8_t channelNb, const uint16_t lookUpTable[])
{
  uint16_t centreAdjust = 0;

  //equation to adjust center of waveform
  centreAdjust = (32767 - ((DACChannel[channelNb].amplitude * 32767) / 100)) + DACChannel[channelNb].offset;


  DACChannel[channelNb].index = (DACChannel[channelNb].index + DACChannel[channelNb].frequency) % 10000;

  //equation to adjust PP voltage of waveform
  return ((lookUpTable[DACChannel[channelNb].index] * (DACChannel[channelNb].amplitude)) / 100) + centreAdjust;
}

uint16_t AWG_DAC_Get(uint8_t channelNb)
{
  switch (DACChannel[channelNb].waveform)
  {
    case (WAVEFORMS_SINEWAVE):
      return VoltageAdjust(channelNb, WAVEFORM_SINEWAVE);
      break;

    case (WAVEFORMS_SQUAREWAVE):
      return VoltageAdjust(channelNb, WAVEFORM_SQUAREWAVE);
      break;

    case (WAVEFORMS_TRIANGLEWAVE):
      return VoltageAdjust(channelNb, WAVEFORM_TRIANGLEWAVE);
      break;

    case (WAVEFORMS_SAWTOOTHWAVE):
      return VoltageAdjust(channelNb, WAVEFORM_SAWTOOTHWAVE);
      break;

  }

}


//Can be used for both offset and amplitude
uint16_t AmplitudeOffsetConversion(uint16_t value)
{
  uint16_t convertedValue;

  convertedValue = (value * 100) / 32767;

  if(!((value * 100) / 32767))
  {
    return convertedValue;
  }

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

  if (!((frequency * 10) % 256))
  {
    return convertedFrequency;
  }

  if (((frequency * 10) % 256) >= 128)
  {
    convertedFrequency++;
  }

  return convertedFrequency;
}

uint16_t Average(uint16_t index, const uint16_t waveform[])
{
  uint16_t addition, difference, value1, value2;

  value1 = waveform[index/10];

  if (index+10 < 100000)
  {
    value2 = waveform[(index+10)/10];
  }
  else
  {
    value2 = value1;
    value1 = waveform[0];
  }

  difference = value2 - value1;

  addition = difference*(index%10)/10;

  return addition + waveform[index/10];
}



/*!
** @}
*/
