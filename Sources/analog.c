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

static const uint8_t Analog_Address = 0x03;

TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

static const uint8_t DAC_Select = 0x01;

static const uint16_t Mid_Scale = 0x8000;

static const uint8_t Span_Code = 0x03;

static const uint32_t All_DACs = 0x0f0000;

static const uint32_t Init_Command = 0x800000;

static const uint32_t Message_Command = 0x700000;

/*! @brief Function to write to the DAC
 *
 *  @param channelNb is the number of the analog input channel to sample.
 *  @param data is data to transmit.
 *
 *  @return bool - true if the data was sent successfully.
 *
 */
bool Analog_Put(const uint32union_t data, const uint8_t channelNb)
{

  PMcL_SPI_Adv_SelectSlaveDevice(4);
  PMcL_SPI_Adv_Exchange(data.s.Hi, Analog_Input[channelNb].putPtr, DAC_Select, TRUE);
  PMcL_SPI_Adv_Exchange(data.s.Lo, Analog_Input[channelNb].putPtr, DAC_Select, FALSE);

  return TRUE;

}

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
  SPIValues.ctar[0].frameSize = 32;
  SPIValues.ctar[1].frameSize = 16;

  for (uint8_t i = 0; i < 2; i++)
  {
    SPIValues.ctar[i].clockPolarity = SPI_CLOCK_POLARITY_INACTIVE_LOW;
    SPIValues.ctar[i].clockPhase = SPI_CLOCK_PHASE_CAPTURED_ON_LEADING;
    SPIValues.ctar[i].firstBit = SPI_FIRST_BIT_MSB;
    SPIValues.ctar[i].delayAfterTransfer = 5000;
    SPIValues.ctar[i].baudRate = 1000000;
  }


  //call SPI_Init
  SPI_Init(&SPIValues, moduleClock);

  //Initalise DAC
  uint16_t data;


  //Set all DACs to -10V to +10V bipolar range
  //Set all DACs to mid-scale
  //Update all DACs for both span and code
  data = Init_Command | All_DACs | Span_Code;
  PMcL_SPI_Adv_Exchange(data, Analog_Input[channelNb].putPtr, DAC_Select, FALSE);
  data = Init_Command | All_DACs | Mid_Scale;
  PMcL_SPI_Adv_Exchange(data, Analog_Input[channelNb].putPtr, DAC_Select, FALSE);

  return TRUE;
}


bool Analog_Get(const uint8_t channelNb)
{
  uint16_t data;

  SPI_SelectSlaveDevice(7);

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

  if (Analog_Input[channelNb].putPtr == &(Analog_Input[channelNb].values[4]))
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  else
    Analog_Input[channelNb].putPtr ++;


  return TRUE;
}

/*!
** @}
*/

