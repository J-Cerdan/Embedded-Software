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
#include "OS.h"


bool FIFO_Init(TFIFO * const fifo)
{
  if (fifo != NULL)
    {
      //Initialisation of END and Start indices and Number of Bytes of FIFO
      fifo->End = 0;
      fifo->Start = 0;
      fifo->NbBytes = OS_SemaphoreCreate(0);
      fifo->BytesAvailable = OS_SemaphoreCreate(FIFO_SIZE);
      return TRUE;
    }

  return FALSE;
}


bool FIFO_Put(TFIFO * const fifo, const uint8_t data)
{
  //OS_SemaphoreWait(fifo->BytesAvailable, 0);
  //Checks if FIFO has reached maximum capacity
  /*if (fifo->NbBytes == FIFO_SIZE)
    {
      return FALSE;
    }*/

  //Critical mode to stop foreground or background operations
  OS_DisableInterrupts();
  //Assigns received data into correct FIFO location
  fifo->Buffer[fifo->End] = data;

  //Maintains End index
  fifo->End++;

  //Checks and resets End index to restrict to certain values
  if (fifo->End > FIFO_SIZE-1)
    fifo->End = 0;
  OS_EnableInterrupts();
  //Maintains Number of Bytes within FIFO
  OS_SemaphoreSignal(fifo->NbBytes);


  return TRUE;
}


bool FIFO_Get(TFIFO * const fifo, uint8_t * const dataPtr)
{
  //OS_SemaphoreWait(fifo->NbBytes, 0);
  //Checks if any data is stored in the FIFO
  /*if (fifo->NbBytes == 0)
    {
      return FALSE;
    }*/

  //Critical mode to stop foreground or background operations
  OS_DisableInterrupts();
  //Identifies oldest data within FIFO and assigns to data pointer for transmission
  *dataPtr = fifo->Buffer[fifo->Start];

  //Maintains Number of Bytes within FIFO
  //fifo->NbBytes--;
  //Maintains Start index
  fifo->Start++;

  //Checks and resets Start index to restrict to certain values
  if (fifo->Start > FIFO_SIZE-1)
    fifo->Start = 0;

  OS_EnableInterrupts();
  OS_SemaphoreSignal(fifo->BytesAvailable);

  return TRUE;

}


/*!
** @}
*/







