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

TAWGDacChannel AWG_DAC_CHANNELS[AWG_NB_CHANNELS];

/*! @brief Sets up the DAC Channels before first use.
 *
 *  @return bool - true if the Channels were was successfully initialized.
 */
bool AWG_Init(void)
{
  for (uint8_t i = 0; i < AWG_NB_CHANNELS; i++)
  {
    AWG_DAC_CHANNELS[i].waveform = SINE_FUNCTION;  	//default wave is sine
    AWG_DAC_CHANNELS[i].frequency = 600; 		//default frequency is 1Hz
    AWG_DAC_CHANNELS[i].amplitude = 10; 		//default amplitude is 1 V
    AWG_DAC_CHANNELS[i].offset = 0; 			//default offset is 0
    AWG_DAC_CHANNELS[i].index = 0;		//index begins at 0 always
    AWG_DAC_CHANNELS[i].active = TRUE;			//remains inactive till PC sends signal
  }

  return TRUE;
}

uint16_t ProcessSample(const uint16 functionLookUpTable[], uint8_t channelNb)
{
  uint16_t value = functionLookUpTable[AWG_DAC_CHANNELS[channelNb].index];
  if (value > 32767)
  {
    value -= 32767;
    value /= 100;
    value *= AWG_DAC_CHANNELS[channelNb].amplitude;
    value += 32767;
  }
  else
  {
    value = 32767 - value;
    value /= 100;
    value *= AWG_DAC_CHANNELS[channelNb].amplitude;
    value = 32767 - value;
  }

  AWG_DAC_CHANNELS[channelNb].index = (AWG_DAC_CHANNELS[channelNb].index + AWG_DAC_CHANNELS[channelNb].frequency) % 10000; //maintain the index count

  return value;
}

uint16_t AWG_SampleGet(uint8_t channelNb)
{
  if (channelNb == 0 || channelNb == 1)
  {
    switch (AWG_DAC_CHANNELS[channelNb].waveform)
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

  AWG_DAC_CHANNELS[channelNb].frequency = (uint16_t)value;

  return TRUE;
}

bool AWG_UpdateAmplitude(uint8_t channelNb, uint16_t amplitude)
{
  uint64_t value = amplitude * 10;

  if ((value % 3276) >= 1638)
  {
    value = (value / 1638) + 1;
  }
  else
    value /= 1638;

  AWG_DAC_CHANNELS[channelNb].amplitude = (uint16_t)value;

  return TRUE;
}



/*!
** @}
*/
