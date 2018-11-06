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

#include "AWG.h"
#include "types.h"
#include "PE_Types.h"
#include "functions.h"

TAWGDacChannel AWG_DAC_Channels[AWG_NB_CHANNELS];

/*! @brief Sets up the DAC Channels before first use.
 *
 *  @return bool - true if the Channels were was successfully initialized.
 */
bool AWG_Init(void)
{
  for (uint8_t i = 0; i < AWG_NB_CHANNELS; i++)
  {
    AWG_DAC_Channels[i].waveform = SINE_FUNCTION;  	//default wave is sine
    AWG_DAC_Channels[i].frequency = 10; 		//default frequency is 1Hz
    AWG_DAC_Channels[i].amplitude = 10; 		//default amplitude is 1 V
    AWG_DAC_Channels[i].offset = 0; 			//default offset is 0
    AWG_DAC_Channels[i].index = 0;			//index begins at 0 always
    AWG_DAC_Channels[i].sizeOfArbitraryWave = 0;	//size of arbitrary array is 0 until wave is uploaded
    AWG_DAC_Channels[i].arbitraryIndexAdder = 0;	//index is 0 until arbitrary wave is uploaded
    AWG_DAC_Channels[i].active = FALSE;			//remains inactive till PC sends signal
  }

  return TRUE;
}

void AWG_ResetAbitraryWave(uint8_t channelNb)
{
  AWG_DAC_Channels[channelNb].sizeOfArbitraryWave = 0;
}

bool AWG_UploadAbitraryWave(int16_t sample, uint8_t channelNb)
{
  AWG_DAC_Channels[channelNb].arbitraryWave[(AWG_DAC_Channels[channelNb].sizeOfArbitraryWave)] = (sample + 32767);
  AWG_DAC_Channels[channelNb].sizeOfArbitraryWave++;
  return TRUE;
}

bool AWG_UpdateArbitraryIndexAdder(uint8_t channelNb)
{
  uint16_t samples = 100000 / AWG_DAC_Channels[channelNb].frequency;

  if (((AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 100) % samples) > (samples / 2))
  {
    (AWG_DAC_Channels[channelNb].arbitraryIndexAdder = (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 100) / samples) + 1;
    AWG_DAC_Channels[channelNb].index = 0;
    return TRUE;
  }
  else
  {
    (AWG_DAC_Channels[channelNb].arbitraryIndexAdder = (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 100) / samples);
    AWG_DAC_Channels[channelNb].index = 0;
    return TRUE;
  }

  return FALSE;
}

uint16_t ApplyAmplitude(uint16_t value, uint8_t channelNb)
{
  uint16_t sample = value;
  if (sample > 32767)
  {
    sample -= 32767;
    sample /= 100;
    sample *= AWG_DAC_Channels[channelNb].amplitude;
    return sample + 32767;
  }
  else
  {
    sample = 32767 - value;
    sample /= 100;
    sample *= AWG_DAC_Channels[channelNb].amplitude;
    return 32767 - sample;
  }
}

uint16_t ApplyOffset(uint16_t value, uint8_t channelNb)
{
  int64_t sample = value + AWG_DAC_Channels[channelNb].offset;

  if (sample > 65534)
    return 65534;
  else if (sample < 0)
    return 0;
  else
    return (uint16)sample;

}


uint16_t ProcessSample(const uint16 functionLookUpTable[], uint8_t channelNb)
{
  uint16_t value = functionLookUpTable[AWG_DAC_Channels[channelNb].index];

  value = ApplyAmplitude(value, channelNb);

  value = ApplyOffset(value, channelNb);
  AWG_DAC_Channels[channelNb].index = (AWG_DAC_Channels[channelNb].index + AWG_DAC_Channels[channelNb].frequency) % 10000; //maintain the index count

  return value;
}

uint16_t ProcessAbitrarySample(const uint16 functionLookUpTable[], uint8_t channelNb)
{
  uint16_t value, moddedValue, sample1, sample2;
  bool sample1Larger;
  if ((AWG_DAC_Channels[channelNb].index % 10) == 0)
  {
    uint16_t value = functionLookUpTable[AWG_DAC_Channels[channelNb].index / 10];

    value = ApplyAmplitude(value, channelNb);

    value = ApplyOffset(value, channelNb);
    AWG_DAC_Channels[channelNb].index = (AWG_DAC_Channels[channelNb].index + AWG_DAC_Channels[channelNb].arbitraryIndexAdder) % (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave*10); //maintain the index count

    return value;
  }
  else
  {
    sample1 = functionLookUpTable[AWG_DAC_Channels[channelNb].index / 10];
    sample2 = functionLookUpTable[(AWG_DAC_Channels[channelNb].index / 10) + 1];

    if (sample1 > sample2)
    {
      value = sample1 - sample2;
      sample1Larger = TRUE;
    }
    else
    {
      value = sample2 - sample1;
      sample1Larger = FALSE;
    }

    value = (value * 10) * (AWG_DAC_Channels[channelNb].index % 10);
    if ((value % 100) >= 50)
    {
      if(sample1Larger)
	value = sample1 - ((value / 100) + 1);
      else
	value = sample1 + ((value / 100) + 1);
    }
    else
    {
      if(sample1Larger)
	value = sample1 - ((value / 100) + 1);
      else
	value = sample1 + ((value / 100) + 1);
    }

    value = ApplyAmplitude(value, channelNb);

    value = ApplyOffset(value, channelNb);
    AWG_DAC_Channels[channelNb].index = (AWG_DAC_Channels[channelNb].index + AWG_DAC_Channels[channelNb].arbitraryIndexAdder) % (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 10); //maintain the index count

    return value;
  }
}

uint16_t AWG_SampleGet(uint8_t channelNb)
{
  if (channelNb == 0 || channelNb == 1)
  {
    switch (AWG_DAC_Channels[channelNb].waveform)
    {
      case (SINE_FUNCTION):
	return ProcessSample(FUNCTIONS_SINEWAVE, channelNb);
	break;

      case (SQUARE_FUNCTION):
	return ProcessSample(FUNCTIONS_SQUAREWAVE, channelNb);
	break;

      case (SAWTOOTH_FUNCTION):
	return ProcessSample(FUNCTIONS_SAWTOOTHWAVE, channelNb);
	break;

      case (TRIANGLE_FUNCTION):
	return ProcessSample(FUNCTIONS_TRIANGLEWAVE, channelNb);
	break;

      case (NOISE):
	return ProcessSample(FUNCTIONS_TRIANGLEWAVE, channelNb);
	break;

      case (ARBITRARY_FUNCTION):
	return ProcessAbitrarySample(AWG_DAC_Channels[channelNb].arbitraryWave, channelNb);
	break;
    }
  }
}

bool AWG_UpdateFrequency(uint8_t channelNb, uint16_t frequency)
{
  uint64_t value = frequency * 10;

  if ((value % 256) >= 128)
  {
    value = (value / 256) + 1;
  }
  else
    value /= 256;

  AWG_DAC_Channels[channelNb].frequency = (uint16_t)value;

  if (AWG_DAC_Channels[channelNb].waveform == ARBITRARY_FUNCTION)
    AWG_UpdateArbitraryIndexAdder(channelNb);

  return TRUE;
}

bool AWG_UpdateAmplitude(uint8_t channelNb, uint16_t amplitude)
{
  if (channelNb == 0 || channelNb == 1)
  {
    uint64_t value = amplitude * 10;

    if ((value % 3276) >= 1638)
    {
      value = (value / 3276) + 1;
    }
    else
      value /= 3276;

    AWG_DAC_Channels[channelNb].amplitude = (uint16_t)value;

    return TRUE;
  }
  return FALSE;
}

bool AWG_UpdateOffset(uint8_t channelNb, uint16_t offset)
{
  if (channelNb == 0 || channelNb == 1)
  {
    AWG_DAC_Channels[channelNb].offset = (uint16_t)offset;
  }
}



/*!
** @}
*/
