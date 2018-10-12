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
#include "ThreadManage.h"

//private transmit and receive FIFO's
static TFIFO TxFIFO, RxFIFO;
//used to store data received to be accessed by thread
static uint8_t RxData;

//stack for thread
OS_THREAD_STACK(UARTRxStack, THREAD_STACK_SIZE);
OS_THREAD_STACK(UARTTxStack, THREAD_STACK_SIZE);
//semaphore used to handle transmitting and receiving data
static OS_ECB* RxTrue;
static OS_ECB* TxTrue;

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

  //create semaphore
  RxTrue = OS_SemaphoreCreate(0);
  TxTrue = OS_SemaphoreCreate(0);

  //create the thread
  OS_ThreadCreate(UARTRxThread, NULL, &UARTRxStack[THREAD_STACK_SIZE - 1], UART_RX_THREAD);
  OS_ThreadCreate(UARTTxThread, NULL, &UARTTxStack[THREAD_STACK_SIZE - 1], UART_TX_THREAD);


  //initialize transmit and receive FIFO and returns 1 if the succeed, marking the success of initializing the UART
  return FIFO_Init(&TxFIFO) &&
	 FIFO_Init(&RxFIFO);
}


bool UART_InChar(uint8_t* const dataPtr)
{
  return FIFO_Get(&RxFIFO, dataPtr);
}


bool UART_OutChar(const uint8_t data)
{
  UART2_C2 |= UART_C2_TIE_MASK;
  return FIFO_Put(&TxFIFO, data);
}

/*
void UART_Poll(void)
{
  //Bug with reading same register twice without acting on it, placing it in local variable to fix
  uint8_t tempRead = UART2_S1;
  if (tempRead & UART_S1_RDRF_MASK)//true if receive register full flag is set
    FIFO_Put(&RxFIFO, UART2_D);

  if (tempRead & UART_S1_TDRE_MASK)//true if transmit register empty flag is set
    FIFO_Get(&TxFIFO, (uint8_t *) &UART2_D); // type cast to fix volatile error
}
*/
void __attribute__ ((interrupt)) UART_ISR(void)
{
  OS_ISREnter();

  uint8_t tempRead = UART2_S1;

  if (tempRead & UART_S1_RDRF_MASK)//true if receive register full flag is set
    {
      UART2_C2 &= ~UART_C2_RIE_MASK; //Turn of interrupt to exit ISR
      OS_SemaphoreSignal(RxTrue); //signal semaphore to enter thread
    }


  if ((UART2_C2 & UART_C2_TIE_MASK) && (UART2_S1 & UART_S1_TDRE_MASK))
    {
      UART2_C2 &= ~UART_C2_TIE_MASK; // turn off interrupt to exit ISR
      (void)OS_SemaphoreSignal(TxTrue); //signal semaphore to enter thread
    }

  OS_ISRExit();
}

static void UARTRxThread(void* arg)
{
  for (;;)
    {
      (void)OS_SemaphoreWait(RxTrue, 0);

      uint8_t tempRead = UART2_S1; //temp read to clear the RDRF register


      FIFO_Put(&RxFIFO, UART2_D);
      UART2_C2 |= UART_C2_RIE_MASK; //enable interrupt again

    }
}

static void UARTTxThread(void* arg)
{
  for (;;)
    {
      (void)OS_SemaphoreWait(TxTrue, 0);

      if (UART2_S1 & UART_S1_TDRE_MASK)
	{
	  FIFO_Get(&TxFIFO, (uint8_t *) &UART2_D); //loads data into D register

	  UART2_C2 |= UART_C2_TIE_MASK; //enable interrupt again
	}
    }
}

/*!
** @}
*/




