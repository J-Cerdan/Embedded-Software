/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-10
 */
/*!
**  @addtogroup UART_module UART module documentation
**  @{
*/
//This header file implements peripheral memory map for MK70F1 processor.
#include "MK70F12.h"
//provides use definitions
#include "PE_Types.h"
//includes the function prototypes to be implemented here and any public variables or constants
#include "UART.h"
//FIFO header file to access the FIFO functions
#include "FIFO.h"
#include "OS.h"

//private transmit and receive FIFO's
static TFIFO TxFIFO, RxFIFO;
//used to store data received to be accessed by thread
static uint8_t RxData;

//stack for thread
static uint32_t UARTRxStack[800];
static uint32_t UARTTxStack[800];
//semaphore used to handle transmitting and receiving data
static OS_ECB* RxTrue;
static OS_ECB* RxMemoryAvailble;
static OS_ECB* TxTrue;
static OS_ECB* TxNbBytes;
//semaphore to handle packets
OS_ECB* IncomingPacket;

static void UARTTxThread(void* arg);
static void UARTRxThread(void* arg);


bool UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //UART2 on
  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
  //PORTE on
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
  //Set portE bit 16 to be Alt 3 (UART2_TX function)
  PORTE_PCR16 |= PORT_PCR_MUX(3);
  //Set portE bit 17 to be Alt 3 (UART2_TX function)
  PORTE_PCR17 |= PORT_PCR_MUX(3);

  //this enables no parity and 8 bit mode (8N1)
  UART2_C1 = 0x00;
  // disable transmitter and receiver while we adjust and assign values
  UART2_C2 &= ~UART_C2_TE_MASK;
  UART2_C2 &= ~UART_C2_RE_MASK;

  uint16_t SBR;
  uint8_t brfa;

  //the sbr is calculated here and due to integer devision the BRFA is not included in the value
  SBR = moduleClk / (16 * baudRate);

  //The BRFD is multiplied here to shift the decimal to the right so we don't
  // loose information about the BFRA, then use modulus to get the remainder (brfa)
  brfa = (moduleClk*2 / baudRate) % 32;

  //assigning the brfa value in C4
  UART2_C4 |= UART_C4_BRFA(brfa);
  //storing the 5 most significant bits of the SBR in BDH
  UART2_BDH |= UART_BDH_SBR(SBR >>8);
  //storing the 8 least significant bits of the SBR in BDL
  UART2_BDL = (uint8_t) SBR;

  //Enable transmitter and receiver
  UART2_C2 |= UART_C2_TE_MASK;
  UART2_C2 |= UART_C2_RE_MASK;

  //enable interrupts
  UART2_C2 |= UART_C2_RIE_MASK;
  UART2_C2 &= ~UART_C2_TIE_MASK;


  NVICISER1 |= NVIC_ISER_SETENA(1 << (49 % 32));
  NVICICPR1 |= NVIC_ICPR_CLRPEND(1 << (49 % 32));

  //create the thread
  OS_ThreadCreate(UARTRxThread, NULL, &UARTRxStack[199], 2);
  OS_ThreadCreate(UARTTxThread, NULL, &UARTTxStack[199], 1);

  //create semaphore
  RxTrue = OS_SemaphoreCreate(0);
  RxMemoryAvailble = OS_SemaphoreCreate(FIFO_SIZE);
  TxTrue = OS_SemaphoreCreate(0);
  TxNbBytes = OS_SemaphoreCreate(0);
  IncomingPacket = OS_SemaphoreCreate(0);


  //initialize transmit and receive FIFO and returns 1 if the succeed, marking the success of initializing the UART
  return FIFO_Init(&TxFIFO) &&
	 FIFO_Init(&RxFIFO);
}


bool UART_InChar(uint8_t* const dataPtr)
{
  if(FIFO_Get(&RxFIFO, dataPtr))
    {
      OS_SemaphoreSignal(RxMemoryAvailble);
      return TRUE;
    }
  return FALSE;
}


bool UART_OutChar(const uint8_t data)
{
  if (FIFO_Put(&TxFIFO, data))
    {
      OS_SemaphoreSignal(TxNbBytes);
      return TRUE;
    }


  return FALSE;
}


void UART_Poll(void)
{
  //Bug with reading same register twice without acting on it, placing it in local variable to fix
  uint8_t tempRead = UART2_S1;
  if (tempRead & UART_S1_RDRF_MASK)//true if receive register full flag is set
    FIFO_Put(&RxFIFO, UART2_D);

  if (tempRead & UART_S1_TDRE_MASK)//true if transmit register empty flag is set
    FIFO_Get(&TxFIFO, (uint8_t *) &UART2_D); // type cast to fix volatile error
}

void __attribute__ ((interrupt)) UART_ISR(void)
{
     OS_ISREnter();
  uint8_t tempRead = UART2_S1;

  if (tempRead & UART_S1_RDRF_MASK)//true if receive register full flag is set
    {
      (void)OS_SemaphoreSignal(RxTrue);
      RxData = UART2_D;
    }


  if ((UART2_C2 & UART_C2_TIE_MASK) && (UART2_S1 & UART_S1_TDRE_MASK))
    {
      UART2_C2 &= ~UART_C2_TIE_MASK; // turn off interrupt to exit ISR
      (void)OS_SemaphoreSignal(TxTrue);
    }
  OS_ISRExit();
}

static void UARTRxThread(void* arg)
{
  for (;;)
    {
      (void)OS_SemaphoreWait(RxTrue, 0);
      (void)OS_SemaphoreWait(RxMemoryAvailble, 0);

      FIFO_Put(&RxFIFO, RxData);
      (void)OS_SemaphoreSignal(IncomingPacket);
    }
}

static void UARTTxThread(void* arg)
{
  for (;;)
    {
      (void)OS_SemaphoreWait(TxNbBytes, 0);
      if (UART2_S1 & UART_S1_TDRE_MASK)//true if transmit register empty flag is set
	{
	  FIFO_Get(&TxFIFO, (uint8_t *) &UART2_D);
	}
      UART2_C2 |= UART_C2_TIE_MASK; //enable transmit interrupt if there is something is the TxFIFO
      (void)OS_SemaphoreWait(TxTrue, 0);
    }
}

/*!
** @}
*/




