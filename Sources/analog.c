/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  This contains the functions for reading analog values from the LTC1859 ADC on the TWR-ADCDAC-LTC board.
 *  The ADC is 16-bit, and configured with a bipolar voltage range of +/- 10V.
 *  This contains the functions for writing analog values to the LTC2704 DAC on the TWR-ADCDAC-LTC board.
 *  The DAC is 16-bit, and configured with a bipolar voltage range of +/- 10V.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-09-19
 */
/*!
**  @addtogroup analog_module analog module documentation
**  @{
*/


// new types
#include "types.h"
#include "analog.h"
#include "PE_Types.h"
#include "SPI.h"
#include "median.h"


//used to determine channel 0 or 1
#define CH_ZERO 0
#define CH_ONE 4

//used to build the data that will be sent to the analog chip
static const uint16_t Channel_Mask = 0x8400;

static const uint8_t Analog_Address = 0x3;

TAnalogInput Analog_Input[ANALOG_NB_INPUTS];


/*! @brief Function to obtain and store median values
 *
 *  @param channelNb is the number of the analog input channel to sample.
 *  @return none.
 */
void getMedian(const uint8_t channelNb)
{
  Analog_Input[channelNb].oldValue = Analog_Input[channelNb].value;
  Analog_Input[channelNb].value.l = Median_Filter(Analog_Input[channelNb].values, (uint32_t) ANALOG_WINDOW_SIZE);
}


bool Analog_Init(const uint32_t moduleClock)
{

  //set the Analog_Input elements to 0;
  for (uint8_t i = 0; i < ANALOG_NB_INPUTS; i++)
    {
      Analog_Input[i].value.l = 0;
      Analog_Input[i].oldValue.l = 0;

      for (uint8_t j = 0; j < ANALOG_WINDOW_SIZE; j++)
      	{
      	  Analog_Input[i].values[j] = 0;
      	}

      Analog_Input[i].putPtr = &Analog_Input[i].values[0];

    }

  //build the TSPIModule struct to send to the SPI_init
  TSPIModule SPIValues;
  SPIValues.isMaster = TRUE;
  SPIValues.continuousClock = FALSE;
  SPIValues.inactiveHighClock = FALSE;
  SPIValues.changedOnLeadingClockEdge = FALSE;
  SPIValues.LSBFirst = FALSE;
  SPIValues.baudRate = 1000000;

  //call SPI_Init
  return SPI_Init(&SPIValues, moduleClock);
}


bool Analog_Get(const uint8_t channelNb)
{
  static uint8_t position0 = 0, position1 = 0;
  //data to send to the analog chip
  uint16_t data;

  SPI_SelectSlaveDevice(3);

  if(channelNb)
    Analog_Input[channelNb].putPtr = &Analog_Input[channelNb].values[position1];
  else
    Analog_Input[channelNb].putPtr = &Analog_Input[channelNb].values[position0];

  //selecting the correct channel
  switch (channelNb)
  {
    case 0: data = Channel_Mask | (CH_ZERO << 12);
      break;

    case 1: data = Channel_Mask | (CH_ONE << 12);
      break;

    default: return FALSE;
  }

  //perform and exchange with the analog chip
  SPI_Exchange(data, Analog_Input[channelNb].putPtr);

  SPI_Exchange(data, Analog_Input[channelNb].putPtr);

  getMedian(channelNb);
  if(channelNb)
    {
      position1++;
      if (position1 == 5)
	position1 = 0;
    }
  else
    {
      position0++;
        if (position0 == 5)
  	position0 = 0;
    }


  return TRUE;
}

/*!
** @}
*/

