/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  This contains the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-10
 */

/*!
**  @addtogroup FIFO_module FIFO module documentation
**  @{
*/
//includes the function prototypes to be implemented here and any public variables or constants
#include "FIFO.h"
//provides useful definitions
#include "PE_Types.h"


bool FIFO_Init(TFIFO * const fifo)
{
  if (fifo != NULL)
    {
      //Initialisation of END and Start indices and Number of Bytes of FIFO
      fifo->End = 0;
      fifo->Start = 0;
      fifo->NbBytes = 0;
      return TRUE;
    }

  return FALSE;
}


bool FIFO_Put(TFIFO * const fifo, const uint8_t data)
{
  //Critical mode to stop foreground or background operations
  EnterCritical();
  //Checks if FIFO has reached maximum capacity
  if (fifo->NbBytes == FIFO_SIZE)
    {
      ExitCritical();
      return FALSE;
    }

  //Assigns received data into correct FIFO location
  fifo->Buffer[fifo->End] = data;

  //Maintains End index
  fifo->End++;

  //Checks and resets End index to restrict to certain values
  if (fifo->End > FIFO_SIZE-1)
    fifo->End = 0;

  //Maintains Number of Bytes within FIFO
  fifo->NbBytes++;

  ExitCritical();
  return TRUE;
}


bool FIFO_Get(TFIFO * const fifo, uint8_t * const dataPtr)
{
  //Critical mode to stop foreground or background operations
  EnterCritical();
  //Checks if any data is stored in the FIFO
  if (fifo->NbBytes == 0)
    {
      ExitCritical();
      return FALSE;
    }


  //Identifies oldest data within FIFO and assigns to data pointer for transmission
  *dataPtr = fifo->Buffer[fifo->Start];

  //Maintains Number of Bytes within FIFO
  fifo->NbBytes--;

  //Maintains Start index
  fifo->Start++;

  //Checks and resets Start index to restrict to certain values
  if (fifo->Start > FIFO_SIZE-1)
    fifo->Start = 0;

  ExitCritical();
  return TRUE;

}


/*!
** @}
*/







