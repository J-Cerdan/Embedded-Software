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




/*! @brief Function to write to calculate value needed for waveform
 *
 *  @param index is the index position of the value required for the function
 *  @param waveform is the waveform array required to obtain the value
 *
 *  @return returns the averaged value of the two values
 *
 */
uint16_t Average(uint16_t index, const uint16_t waveform)
{
  num1 = waveform[index/10];

  if (index+10 < 100000)
    {
      num2 = waveform[(index+10)/10];
    }
  else
    {
      num2 = num1;
      num1 = waveform[0];
    }

  difference = num2 - num1;

  addition = difference*(index%10)/10;

  return addition + waveform[index/10];
}

/*!
** @}
*/
