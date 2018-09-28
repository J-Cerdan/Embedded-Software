/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  This contains the functions for reading analog values from the LTC1859 ADC on the TWR-ADCDAC-LTC board.
 *  The ADC is 16-bit, and configured with a bipolar voltage range of +/- 10V.
 *  This contains the functions for writing analog values to the LTC2704 DAC on the TWR-ADCDAC-LTC board.
 *  The DAC is 16-bit, and configured with a bipolar voltage range of +/- 10V.
 *
 *  @author PMcL
 *  @date 2016-09-23
 */



// new types
#include "types.h"
#include "analog.h"
#include "PE_Types.h"
#include "SPI.h"



#define CH_ZERO 0
#define CH_TWO 1
#define CH_FOUR 2
#define CH_SIX 3
#define CH_ONE 4
#define CH_THREE 5
#define CH_FIVE 6
#define CH_SEVEN 7


static const uint8_t Channel_Mask = 0x84;

/*
 * {
  int16union_t value;                  !< The current "processed" analog value (the user updates this value).
  int16union_t oldValue;               !< The previous "processed" analog value (the user updates this value).
  int16_t values[ANALOG_WINDOW_SIZE];  !< An array of sample values to create a "sliding window".
  int16_t* putPtr;                     !< A pointer into the array of the last sample taken.
} TAnalogInput
 */

TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

/*! @brief Sets up the ADC before first use.
 *
 *  @param moduleClock The module clock rate in Hz.
 *  @return bool - true if the UART was successfully initialized.
 */
bool Analog_Init(const uint32_t moduleClock)
{
  TSPIModule SPIValues;
  SPIValues.isMaster = TRUE;
  SPIValues.continuousClock = FALSE;
  SPIValues.inactiveHighClock = FALSE;
  SPIValues.changedOnLeadingClockEdge = TRUE;
  SPIValues.LSBFirst = FALSE;
  SPIValues.baudRate = 1000000;

  return SPI_Init(&SPIValues, moduleClock);
}

/*! @brief Takes a sample from an analog input channel.
 *
 *  @param channelNb is the number of the analog input channel to sample.
 *  @return bool - true if the channel was read successfully.
 */
bool Analog_Get(const uint8_t channelNb)
{
  //1 _ _ _ 0 1 0 0 0x84
  uint8_t data;

  switch (channelNb)
  {
    case 0: data = Channel_Mask | (CH_ZERO << 4);
      break;

    case 1: data = Channel_Mask | (CH_ONE << 4);
      break;

    case 2: data = Channel_Mask | (CH_TWO << 4);
      break;

    case 3: data = Channel_Mask | (CH_THREE << 4);
      break;

    case 4: data = Channel_Mask | (CH_FOUR << 4);
      break;

    case 5: data = Channel_Mask | (CH_FIVE << 4);
      break;

    case 6: data = Channel_Mask | (CH_SIX << 4);
      break;

    case 7: data = Channel_Mask | (CH_SEVEN << 4);
      break;

    default: return FALSE;
  }

  SPI_Exchange((uint16_t) data, Analog_Input[channelNb].putPtr);

  return TRUE;
}

