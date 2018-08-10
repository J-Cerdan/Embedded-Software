/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-10
 */
//This header file implements peripheral memory map for MK70F1 processor.
#include "MK70F12.h"
//provides use definitions
#include "PE_Types.h"
//includes the function prototypes to be implemented here and any public variables or constants
#include "UART.h"
//FIFO header file to access the FIFO functions
#include "FIFO.h"

//private transmit and receive FIFO's
static TFIFO TxFIFO, RxFIFO;
/*! @brief Sets up the UART interface before first use.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *  @return bool - TRUE if the UART was successfully initialized.
 */
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
  UART2_C2 &= UART_C2_TE_MASK;
  UART2_C2 &= UART_C2_RE_MASK;

  uint16_t SBR;
  uint8_t brfa;

  //the sbr is calculated here and due to integer devision the BRFA is not included in the value
  SBR = moduleClk / (16 * baudRate);

  //The BRFD is multiplied here to shift the decimal to the right so we don't loose information about the BFRA, then use modulus to get the remainder (brfa)
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

  //initialize transmit and receive FIFO and returns 1 if the succeed, marking the success of initializing the UART
  return FIFO_Init(&TxFIFO) &&
	 FIFO_Init(&RxFIFO);
}

/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return bool - TRUE if the receive FIFO returned a character.
 *  @note Assumes that UART_Init has been called.
 */
bool UART_InChar(uint8_t* const dataPtr)
{
  return FIFO_Get(&RxFIFO, dataPtr);
}

/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return bool - TRUE if the data was placed in the transmit FIFO.
 *  @note Assumes that UART_Init has been called.
 */
bool UART_OutChar(const uint8_t data)
{
  return FIFO_Put(&TxFIFO, data);
}

/*! @brief Poll the UART status register to try and receive and/or transmit one character.
 *
 *  @return void
 *  @note Assumes that UART_Init has been called.
 */
void UART_Poll(void)
{
  //Bug with reading same register twice without acting on it, placing it in local variable to fix
  uint8_t tempRead = UART2_S1;
  if (tempRead & UART_S1_RDRF_MASK)//true if recieve register full flag is set
    FIFO_Put(&RxFIFO, UART2_D);

  if (tempRead & UART_S1_TDRE_MASK)//true if transmit register empty flag is set
    FIFO_Get(&TxFIFO, (uint8_t *) &UART2_D); // type cast to fix volatile error
}






