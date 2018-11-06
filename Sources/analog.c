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
#include "median.h"
#include "PMcL_SPI_Adv.h"


//used to determine channel 0 or 1
#define CH_ZERO 0
#define CH_ONE 4

//used to build the data that will be sent to the analog chip
static const uint16_t Channel_Mask = 0x8400;

static const uint8_t Analog_Address = 0x03;

TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

static const uint8_t DAC_Select = 0x01;

//Initialise Command to all DACs to Mid_Scale with Span
static uint32union_t Init_Command;

//Message to either channel 0 or 1
static const uint16_t Message_Command[] = {0x0070, 0x0072};


bool Analog_Put(const uint16_t data, const uint8_t channelNb)
{
  // Select LTC2704
  PMcL_SPI_Adv_SelectSlaveDevice(4);

  if (channelNb == 0 || channelNb == 1)
  {
    PMcL_SPI_Adv_Exchange(Message_Command[channelNb], NULL, DAC_Select, TRUE);
    PMcL_SPI_Adv_Exchange(data, NULL, DAC_Select, FALSE);
  }
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

  //Initalise DAC
  //Set all DACs to -10V to +10V bipolar range 0x03
  //Set all DACs to mid-scale 0x8000
  //Update all DACs for both span and code 0x008f
  Init_Command.l = 0x008f8003;

  // Select LTC2704
  PMcL_SPI_Adv_SelectSlaveDevice(4);

  //Sends initialisation codes for DAC
  PMcL_SPI_Adv_Exchange(Init_Command.s.Hi, NULL, DAC_Select, TRUE);
  PMcL_SPI_Adv_Exchange(Init_Command.s.Lo, NULL, DAC_Select, FALSE);

  return TRUE;
}


bool Analog_Get(const uint8_t channelNb)
{
  uint16_t data;

  PMcL_SPI_Adv_SelectSlaveDevice(7);

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

