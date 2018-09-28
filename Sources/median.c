/*! @file
 *
 *  @brief Median filter.
 *
 *  This contains the functions for performing a median filter on half-word-sized data.
 *
 *  @author PMcL
 *  @date 2015-10-12
 */


// New types
#include "types.h"

/*! @brief Median filters half-words.
 *
 *  @param array is an array half-words for which the median is sought.
 *  @param size is the length of the array.
 */
int16_t Median_Filter(const int16_t array[], const uint32_t size)
{
  //copy the array
  int16_t sortedArray[size];
  for (uint32_t z = 0; z < size; z++)
    {
      sortedArray[z] = sortedArray[z];
    }

  //sort it in accending order
  int16_t temp;
  for (uint32_t i = 0; i < size; i++)
    {
      for (uint32_t j = 1 + i; j < size; j++)
	{
	  if (sortedArray[i] > sortedArray[j])
	    {
	      temp = sortedArray[i];
	      sortedArray[i] = sortedArray[j];
	      sortedArray[j] = temp;
	    }
	}
    }

  //return the median depending on where the array is even or odd sized
  if (size % 2)
    {
      return sortedArray[size / 2];
    }
  else
    {
      return (sortedArray[size / 2] + sortedArray[(size / 2)-1]) / 2;
    }
}

