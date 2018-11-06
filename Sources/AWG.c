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

//Arbitrary Index Counter
uint16_t ArbIndex = 0;
//Edit Channel Selection
static uint8_t EditChannel = 0;

TAWGChannel DACChannel[NB_DAC_CHANNELS];

bool AWG_Init(void)
{
  for (uint8_t i=0; i < NB_DAC_CHANNELS; i++)
  {
    DACChannel[i].active = FALSE;
    DACChannel[i].waveform = WAVEFORMS_SINEWAVE;
    DACChannel[i].frequency = 10;
    DACChannel[i].amplitude = 10;
    DACChannel[i].offset = 0;
    DACChannel[i].index = 0;
    DACChannel[i].arbitrarymaxindex = 0;
    DACChannel[i].arbitraryincrement = 0;
  }
  return TRUE;
}

/*! @brief Function process to adjust waveform value received
 *
 *  @param channelNb is the amplitude received to be converted
 *  @param value is the amplitude received to be converted
 *
 *  @return returns the amplitude in correct form
 *
 */
static int32_t VoltageAdjust(uint8_t channelNb, const uint16_t lookUpTable[])
{
  int16_t centreAdjust = 0;

  //equation to adjust center of waveform
  centreAdjust = (32767 - ((DACChannel[channelNb].amplitude * 32767) / 100)) + DACChannel[channelNb].offset;

  //Index increment depending on configured frequency and waveform
  if (DACChannel[channelNb].waveform == WAVEFORMS_ARBITRARYWAVE)
  {
    DACChannel[channelNb].index = (DACChannel[channelNb].index + DACChannel[channelNb].frequency) % DACChannel[channelNb].arbitrarymaxindex;
  }
  else
  {
    DACChannel[channelNb].index = (DACChannel[channelNb].index + DACChannel[channelNb].frequency) % 10000;
  }

  //return value from equation to adjust values of waveform according to voltage and frequency
  return ((lookUpTable[DACChannel[channelNb].index] * (DACChannel[channelNb].amplitude)) / 100) + centreAdjust;
}

int32_t AWG_DAC_Get(uint8_t channelNb)
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

    case (WAVEFORMS_ARBITRARYWAVE):
      return VoltageAdjust(channelNb, DACChannel[channelNb].arbitrarywaveform);
      break;

  }

}

void Arbitrary_AddValues(uint8_t channelNb, int16_t value)
{
  value += 32767;
  DACChannel[channelNb].arbitrarywaveform[ArbIndex] = value;
  ArbIndex++;
}

/*! @brief Function to convert amplitude value needed for waveform
 *
 *  @param value is the amplitude received to be converted
 *
 *  @return void
 *
 */
static uint16_t amplitudeConversion(uint16_t value)
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

/*! @brief Function to convert frequency value needed for waveform
 *
 *  @param frequency - is the frequency received to be converted
 *
 *  @return returns the frequency in correct form
 *
 */
static uint16_t frequencyConversion(uint16_t frequency)
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

bool Set_Status(uint8_t outputChannel, uint8_t enable)
{
  DACChannel[outputChannel].active = enable;
  return TRUE;
}

bool Set_Waveform(uint8_t selectedWaveform)
{
  DACChannel[EditChannel].waveform = selectedWaveform;
  return TRUE;
}

bool Set_Frequency(uint8_t LSB, uint8_t MSB)
{
  uint16union_t recievedFrequency;
  recievedFrequency.s.Lo = LSB;
  recievedFrequency.s.Hi = MSB;

  DACChannel[EditChannel].frequency = frequencyConversion(recievedFrequency.l);

  return TRUE;
}

bool Set_Amplitude(uint8_t LSB, uint8_t MSB)
{
  uint16union_t recievedAmplitude;
  recievedAmplitude.s.Lo = LSB;
  recievedAmplitude.s.Hi = MSB;

  DACChannel[EditChannel].amplitude = amplitudeConversion(recievedAmplitude.l);

  return TRUE;
}

bool Set_Offset(uint8_t LSB, uint8_t MSB)
{
  int16union_t recievedOffset;
  recievedOffset.s.Lo = LSB;
  recievedOffset.s.Hi = MSB;

  DACChannel[EditChannel].offset = recievedOffset.l;

  return TRUE;
}

bool Set_AllWaveformStatus(bool enable)
{
  for (uint8_t i = 0; i < NB_DAC_CHANNELS; i++)
    DACChannel[i].active = enable;

  return TRUE;
}

bool Set_Active(uint8_t outputChannel)
{
  EditChannel = outputChannel;
  return TRUE;
}


/*!
** @}
*/
