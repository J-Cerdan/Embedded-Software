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
#include "PMcL_SPI_Adv.h"
#include "median.h"


//used to determine channel 0 or 1
#define CH_ZERO 0
#define CH_ONE 4

static const uint8_t ADC_CTAS = 0;
static const uint8_t DAC_CTAS = 1;

//used to build the data that will be sent to the analog chip
static const uint16_t ADC_Mask = 0x8400;
static uint16_t DAC_Command[] = {0x0070, 0x0072};


static const uint8_t Analog_Address = 0x7;
//command to send to the DAC to set up the span for all channels
static uint32union_t DACSetupCommand;

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
  DACSetupCommand.l = 0x008F0003;

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
  SPIValues.ctar[0].frameSize = 16;
  SPIValues.ctar[1].frameSize = 32;
  SPIValues.ctar[0].delayAfterTransfer = 5000;
  SPIValues.ctar[1].delayAfterTransfer = 0;
  for (uint8_t i = 0; i < 2; i++)
  {
    SPIValues.ctar[i].clockPolarity = SPI_CLOCK_POLARITY_INACTIVE_LOW;
    SPIValues.ctar[i].clockPhase = SPI_CLOCK_PHASE_CAPTURED_ON_LEADING;
    SPIValues.ctar[i].firstBit = SPI_FIRST_BIT_MSB;
    SPIValues.ctar[i].baudRate = 1000000;
  }

  //call SPI_Init
  PMcL_SPI_Adv_Init(&SPIValues, moduleClock);

  PMcL_SPI_Adv_SelectSlaveDevice(4);

  //set up all the DAC channel spans
  PMcL_SPI_Adv_Exchange(DACSetupCommand.s.Hi, NULL, DAC_CTAS, TRUE);
  PMcL_SPI_Adv_Exchange(DACSetupCommand.s.Lo, NULL, DAC_CTAS, FALSE);

  return TRUE;
}


bool Analog_Get(const uint8_t channelNb)
{
  uint16_t data;

  PMcL_SPI_Adv_SelectSlaveDevice(7);

  //selecting the correct channel
  switch (channelNb)
  {
    case 0: data = ADC_Mask | (CH_ZERO << 12);
      break;

    case 1: data = ADC_Mask | (CH_ONE << 12);
      break;

    default: return FALSE;
  }

  //perform and exchange with the analog chip
  //SPI_Exchange(data, Analog_Input[channelNb].putPtr);

  //SPI_Exchange(data, Analog_Input[channelNb].putPtr);

  getMedian(channelNb);

  if (Analog_Input[channelNb].putPtr == &(Analog_Input[channelNb].values[4]))
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  else
    Analog_Input[channelNb].putPtr ++;


  return TRUE;
}

bool Analog_Put(const uint8_t channelNb, const uint16_t data)
{
  PMcL_SPI_Adv_SelectSlaveDevice(4); //select the slave
  if (channelNb == 0 || channelNb == 1) //make sure a valid channel is chosen
  {
    PMcL_SPI_Adv_Exchange(DAC_Command[channelNb], NULL, DAC_CTAS, TRUE); //send data to the dac
    PMcL_SPI_Adv_Exchange(data, NULL, DAC_CTAS, FALSE);
    return TRUE;
  }
  return FALSE;
}

/*!
** @}
*/

