/*! @file
 *
 *  @brief Implementation  routines to implement a FIFO buffer.
 *
 *  This contains the implementation of the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-02
 */

#include "FIFO.h"

/*! @brief Initialize the FIFO before first use.
 *
 *  @param fifo A pointer to the FIFO that needs initializing.
 *  @return bool - TRUE if the FIFO was successfully initialised
 */
bool FIFO_Init(TFIFO * const fifo)
{
  fifo->End = 0;
  fifo->Start = 0;
  fifo->NbBytes = 0;
  return TRUE;
}

/*! @brief Put one character into the FIFO.
 *
 *  @param fifo A pointer to a FIFO struct where data is to be stored.
 *  @param data A byte of data to store in the FIFO buffer.
 *  @return bool - TRUE if data is successfully stored in the FIFO.
 *  @note Assumes that FIFO_Init has been called.
 */
bool FIFO_Put(TFIFO * const fifo, const uint8_t data)
{
  if (fifo->NbBytes == FIFO_SIZE)
    return FALSE;

  fifo->Buffer[fifo->End] = data;
  fifo->End++;
  if (fifo->End > FIFO_SIZE-1)
    fifo->End = 0;
  fifo->NbBytes++;

  return TRUE;
}

/*! @brief Get one character from the FIFO.
 *
 *  @param fifo A pointer to a FIFO struct with data to be retrieved.
 *  @param dataPtr A pointer to a memory location to place the retrieved byte.
 *  @return bool - TRUE if data is successfully retrieved from the FIFO.
 *  @note Assumes that FIFO_Init has been called.
 */
bool FIFO_Get(TFIFO * const fifo, uint8_t * const dataPtr)
{
  if (fifo->NbBytes == 0)
    return FALSE;

  *dataPtr = fifo->Buffer[fifo->Start];
  fifo->NbBytes--;
  fifo->Start++;
  if (fifo->Start > FIFO_SIZE-1)
    fifo->Start = 0;

  return TRUE;

}










