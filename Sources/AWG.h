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

#include "types.h"

#ifndef SOURCES_AWG_H_
#define SOURCES_AWG_H_

//macros defined for determining DAC channels
#define NB_DAC_CHANNELS 2
//macros defined for determining Maximum Arbitrary Size
#define MAX_ARBITRARY_SIZE 1000

//Arbitrary Index Counter
extern uint16_t ArbIndex;

//typedef definition structure for waveforms
typedef enum
{
  WAVEFORMS_SINEWAVE = 0,
  WAVEFORMS_SQUAREWAVE = 1,
  WAVEFORMS_TRIANGLEWAVE = 2,
  WAVEFORMS_SAWTOOTHWAVE = 3,
  WAVEFORMS_ARBITRARYWAVE = 5
} TWaveforms;

//Channel Structure
typedef struct
{
  bool active;		/*!< Active Configuration */
  TWaveforms waveform;     /*!< Waveform Configuration */
  uint16_t frequency;    /*!< Frequency Configuration */
  uint8_t amplitude;    /*!< Amplitude Configuration */
  int16_t offset;      /*!< Offset Configuration */
  uint16_t index;	/*!< Index Configuration */
  uint16_t arbitrarymaxindex;	/*!< Arbitrary Maximum Configuration */
  uint16_t arbitraryincrement;	/*!< Arbitrary Increment Configuration */
  uint16_t arbitrarywaveform[MAX_ARBITRARY_SIZE]; /*!< Arbitrary Waveform Array Configuration */

} TAWGChannel;

extern TAWGChannel DACChannel[NB_DAC_CHANNELS];

/*! @brief Function to initialise DAC Channels
 *
 *  @param void
 *
 *  @return bool - TRUE if all the functions that were called were successful
 *
 */
bool AWG_Init(void);

/*! @brief Function to obtain waveform values
 *
 *  @param channelNb channel being used to access data
 *
 *  @return uint32_t the resulting value after processing
 *
 */
int32_t AWG_DAC_Get(uint8_t channelNb);

/*! @brief Function to convert Arbitrary waveform values from signed to unsigned
 *
 *  @param channelNb is channel configured to access data
 *
 *  @return void
 *
 */
void Arbitrary_AddValues(uint8_t channelNb, int16_t value);

/*! @brief Sets active status of one channel
 *
 *  @param outputChannel - channel to be configured
 *  @param enable - status to configure active variable
 *  @return bool - TRUE if all the functions that were called were successful
 */
bool Set_Status(uint8_t outputChannel, uint8_t enable);

/*! @brief Sets waveform of channel being configured
 *
 *  @param selectedWaveform - waveform being selected
 *  @return bool - TRUE if all the functions that were called were successful
 */
bool Set_Waveform(uint8_t selectedWaveform);

/*! @brief Sets frequency of channel being configured
 *
 *  @param LSB
 *  @param MSB
 *  @return bool - TRUE if all the functions that were called were successful
 */
bool Set_Frequency(uint8_t LSB, uint8_t MSB);

/*! @brief Sets amplitude of channel being configured
 *
 *  @param LSB
 *  @param MSB
 *  @return bool - TRUE if all the functions that were called were successful
 */
bool Set_Amplitude(uint8_t LSB, uint8_t MSB);

/*! @brief Sets offset of channel being configured
 *
 *  @param LSB
 *  @param MSB
 *  @return bool - TRUE if all the functions that were called were successful
 */
bool Set_Offset(uint8_t LSB, uint8_t MSB);

/*! @brief Sets all channel active statuses
 *
 *  @param enable - boolean to set statuses of all channels
 *  @return bool - TRUE if all the functions that were called were successful
 */
bool Set_AllWaveformStatus(bool enable);

/*! @brief Sets channel that will be configured into a global variable
 *
 *  @param outputChannel - channel selected to have values edited
 *  @return bool - TRUE if all the functions that were called were successful
 */
bool Set_Active(uint8_t outputChannel);



#endif /* SOURCES_AWG_H_ */

/*!
** @}
*/
