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
#include "MK70F12.h"

TAWGDacChannel AWG_DAC_Channels[AWG_NB_CHANNELS];

/*! @brief Sets up the DAC Channels before first use.
 *
 *  @return bool - true if the Channels were was successfully initialized.
 */
bool AWG_Init(void)
{
  for (uint8_t i = 0; i < AWG_NB_CHANNELS; i++) //initialise values.
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

  //initialising the RNG register to be able to generate random values.
  SIM_SCGC3 |= SIM_SCGC3_RNGA_MASK;
  RNG_CR = RNG_CR_GO_MASK | RNG_CR_HA_MASK | RNG_CR_INTM_MASK; // Enable Go, High Assurance, and Interrupt Mask.

  return TRUE;
}

void AWG_ResetAbitraryWaveSize(uint8_t channelNb)
{
  AWG_DAC_Channels[channelNb].sizeOfArbitraryWave = 0; //reset the size to 0 when reading in a new wave
}

bool AWG_UploadAbitraryWave(int16_t sample, uint8_t channelNb)
{
  //store sample in the array and convert it to unsigned int
  AWG_DAC_Channels[channelNb].arbitraryWave[(AWG_DAC_Channels[channelNb].sizeOfArbitraryWave)] = (sample + 32767);
  AWG_DAC_Channels[channelNb].sizeOfArbitraryWave++; //increment the size of the array
  return TRUE;
}

bool AWG_UpdateArbitraryIndexAdder(uint8_t channelNb)
{
  uint16_t samples = 100000 / AWG_DAC_Channels[channelNb].frequency; //ready the number of samples per sycle

  if (((AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 100) % samples) > (samples / 2)) //round up
  {
    (AWG_DAC_Channels[channelNb].arbitraryIndexAdder = (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 100) / samples) + 1;
    AWG_DAC_Channels[channelNb].index = 0; //reset index to 0 to start form beginning
    return TRUE;
  }
  else //round down
  {
    (AWG_DAC_Channels[channelNb].arbitraryIndexAdder = (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 100) / samples);
    AWG_DAC_Channels[channelNb].index = 0; //reset index to 0 to start form beginning
    return TRUE;
  }

  return FALSE;
}

/*! @brief The value is adjusted according to what the amlitude of the channel is.
 *
 *  @param value is the value to be adjusted to the amplitude
 *  @param channelNb is the channel who's amplitude is to be changed.
 *  @return uint16_t - the value of the sample that has been amplitude adjusted
 */
uint16_t ApplyAmplitude(uint16_t value, uint8_t channelNb)
{
  uint16_t sample = value;
  if (sample > 32767) //for values above the "zero" line
  {
    sample -= 32767;
    sample /= 100;
    sample *= AWG_DAC_Channels[channelNb].amplitude;
    return sample + 32767;
  }
  else //for values below the "zero" line
  {
    sample = 32767 - value;
    sample /= 100;
    sample *= AWG_DAC_Channels[channelNb].amplitude;
    return 32767 - sample;
  }
}

/*! @brief The value is adjusted according to what the offset of the channel is.
 *
 *  @param value is the value to be adjusted to the offset
 *  @param channelNb is the channel who's offset is to be changed.
 *  @return uint16_t - the value of the sample that has been offset adjusted
 */
uint16_t ApplyOffset(uint16_t value, uint8_t channelNb)
{
  int64_t sample = value + AWG_DAC_Channels[channelNb].offset;

  if (sample > 65534) //for over flow to just clamp the value
    return 65534;
  else if (sample < 0) //for values less than 0 to clamp it
    return 0;
  else
    return (uint16)sample; //return the offset value if no overflow or value under zero
}

/*! @brief Process a sample according to the waveform and return it to send.
 *
 *  @param functionLookUpTable is the look up table of the waveform
 *  @param channelNb is the channel that will be processed to generate a sample.
 *  @return uint16_t - the value of the sample that has been processed
 */
uint16_t ProcessSample(const uint16 functionLookUpTable[], uint8_t channelNb)
{
  uint16_t value = functionLookUpTable[AWG_DAC_Channels[channelNb].index]; //grab the value from the look up table

  value = ApplyAmplitude(value, channelNb); //apply the amplitude to the sample

  value = ApplyOffset(value, channelNb); //apply the offset to the sample

  AWG_DAC_Channels[channelNb].index = (AWG_DAC_Channels[channelNb].index + AWG_DAC_Channels[channelNb].frequency) % 10000; //maintain the index count

  return value;
}

/*! @brief Process a sample according to the arbitrary waveform and return it to send.
 *
 *  @param functionLookUpTable is the look up table of the arbitrary waveform
 *  @param channelNb is the channel that will be processed to generate a sample.
 *  @return uint16_t - the value of the sample that has been processed
 */
uint16_t ProcessAbitrarySample(const uint16 functionLookUpTable[], uint8_t channelNb)
{
  uint16_t value, moddedValue, sample1, sample2;
  bool sample1Larger; //to determine if sample one is larger
  if ((AWG_DAC_Channels[channelNb].index % 10) == 0) //for values that are not "decimals"
  {
    uint16_t value = functionLookUpTable[AWG_DAC_Channels[channelNb].index / 10];

    value = ApplyAmplitude(value, channelNb);

    value = ApplyOffset(value, channelNb);
    AWG_DAC_Channels[channelNb].index = (AWG_DAC_Channels[channelNb].index + AWG_DAC_Channels[channelNb].arbitraryIndexAdder) % (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave*10); //maintain the index count

    return value;
  }
  else //for values that are "decimals"
  {
    sample1 = functionLookUpTable[AWG_DAC_Channels[channelNb].index / 10];
    sample2 = functionLookUpTable[(AWG_DAC_Channels[channelNb].index / 10) + 1];

    //find the difference
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

    //interpolation of the value to find the closest value
    value = (value * 10) * (AWG_DAC_Channels[channelNb].index % 10);
    if ((value % 100) >= 50) //for rounding up
    {
      if(sample1Larger) //minus or addition based on of sample one is larger or smaller
	value = sample1 - ((value / 100) + 1);
      else
	value = sample1 + ((value / 100) + 1);
    }
    else
    {
      if(sample1Larger) //minus or addition based on of sample one is larger or smaller
	value = sample1 - ((value / 100) + 1);
      else
	value = sample1 + ((value / 100) + 1);
    }

    value = ApplyAmplitude(value, channelNb); //apply the amplitude

    value = ApplyOffset(value, channelNb); //apply the offset

    AWG_DAC_Channels[channelNb].index = (AWG_DAC_Channels[channelNb].index + AWG_DAC_Channels[channelNb].arbitraryIndexAdder) % (AWG_DAC_Channels[channelNb].sizeOfArbitraryWave * 10); //maintain the index count

    return value;
  }
}

/*! @brief Read a Register that generated a random number.
 *
 *  @return uint16_t - the value of the random number
 */
static uint16_t intRand(void)
{
  while (!(RNG_SR & RNG_SR_OREG_LVL_MASK))
    {/* Wait for a valid output*/}

  return RNG_OR & 0xFFFF;
}

/*! @brief Process a sample that will be used for noise.
 *
 *  @return uint16_t - the value of the noise sample that has been processed
 */
uint16 GetNoiseSample(void)
{
  uint16_t sumOfRandomValues;		//add 12 sets of random noise
  for (uint8_t i = 0; i < 12; i++)
  {
    sumOfRandomValues += intRand();
  }

  return sumOfRandomValues - (4 << 13);	//bit shift the four 13 times and minus it from the random values

}

/*! @brief Request for a sample to get processed according to the channel selected.
 *
 *  @param channelNb is the channel that will processed to generate a sample.
 *  @return uint16_t - the value of the sample that has been processed
 */
uint16_t AWG_SampleGet(uint8_t channelNb)
{
  if (channelNb == 0 || channelNb == 1) //check if a valid channel has been selected
  {
    switch (AWG_DAC_Channels[channelNb].waveform) //switch accroding to thewave form selected by the channel
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
	return GetNoiseSample();
	break;

      case (ARBITRARY_FUNCTION):
	return ProcessAbitrarySample(AWG_DAC_Channels[channelNb].arbitraryWave, channelNb);
	break;
    }
  }
}

/*! @brief Update the frequency of any channel.
 *
 *  @param channelNb is the channel that will have its frequency updated
 *  @param frequency the new frequency value to be updated
 *  @return bool - true if the frequency was updated correctly
 */
bool AWG_UpdateFrequency(uint8_t channelNb, uint16_t frequency)
{
  uint64_t value = frequency * 10;

  if (channelNb == 0 || channelNb == 1) //check to see if valid channels are passe din
  {
    if ((value % 256) >= 128)   //rounds up if true
    {
      value = (value / 256) + 1;
    }
    else
      value /= 256;

    AWG_DAC_Channels[channelNb].frequency = (uint16_t)value; //update the frequency with the new value

    if (AWG_DAC_Channels[channelNb].waveform == ARBITRARY_FUNCTION) //if wave form is arbitrary, update the index addition amount
      AWG_UpdateArbitraryIndexAdder(channelNb);
    return TRUE;
  }

  return FALSE;
}

/*! @brief Update the amplitude of any channel.
 *
 *  @param channelNb is the channel that will have its amplitude updated
 *  @param amplitude the new amplitude value to be updated
 *  @return bool - true if the amplitude was updated correctly
 */
bool AWG_UpdateAmplitude(uint8_t channelNb, uint16_t amplitude)
{
  if (channelNb == 0 || channelNb == 1) //check to see if valid channels are passed in
  {
    uint64_t value = amplitude * 10;

    if ((value % 3276) >= 1638) //rounds up if true
    {
      value = (value / 3276) + 1;
    }
    else
      value /= 3276;

    AWG_DAC_Channels[channelNb].amplitude = (uint16_t)value; //update the amplitude value

    return TRUE;
  }
  return FALSE;
}

/*! @brief Update the offset of any channel.
 *
 *  @param channelNb is the channel that will have its offset updated
 *  @param amplitude the new offset value to be updated
 *  @return bool - true if the offset was updated correctly
 */
bool AWG_UpdateOffset(uint8_t channelNb, uint16_t offset)
{
  if (channelNb == 0 || channelNb == 1) //check to see if a valid channel is provided
  {
    AWG_DAC_Channels[channelNb].offset = (uint16_t)offset;
    return TRUE;
  }
  return FALSE;
}



/*!
** @}
*/
