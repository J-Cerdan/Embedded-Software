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
  return TRUE;
}

