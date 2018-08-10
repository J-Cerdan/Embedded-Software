/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  This contains the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-10
 */

#include "FIFO.h"

/*! @brief Initialize the FIFO before first use.
 *
 *  @param fifo A pointer to the FIFO that needs initializing.
 *  @return bool - TRUE if the FIFO was successfully initialised
 */
bool FIFO_Init(TFIFO * const fifo)
{
  //Initialisation of END, Start and Number of Bytes FIFO indices
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

  //Checks if FIFO reaches maximum capacity
  if (fifo->NbBytes == FIFO_SIZE)
    return FALSE;

  //Assigns received data into correct FIFO location
  fifo->Buffer[fifo->End] = data;

  //Maintains End index
  fifo->End++;

  //Checks and resets End index to restrict to certain values
  if (fifo->End > FIFO_SIZE-1)
    fifo->End = 0;

  //Maintains Number of Bytes within FIFO
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

  //Checks if any data is contained within the FIFO
  if (fifo->NbBytes == 0)
    return FALSE;

  //Identifies oldest data within FIFO and assigns to data pointer for transmission
  *dataPtr = fifo->Buffer[fifo->Start];

  //Maintains Number of Bytes within FIFO
  fifo->NbBytes--;

  //Maintains Start index
  fifo->Start++;

  //Checks and resets Start index to restrict to certain values
  if (fifo->Start > FIFO_SIZE-1)
    fifo->Start = 0;

  return TRUE;

}










