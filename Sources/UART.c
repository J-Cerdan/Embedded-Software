/*! @file
 *
 *  @brief Implementation  I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the implementation of the functions for operating the UART (serial port).
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-07
 */

#include "UART.h"

TFIFO TxFIFO, RxFIFO;
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
  PORTE_PCR16 = PORT_PCR_MUX(3);
  //Set portE bit 17 to be Alt 3 (UART2_TX function)
  PORTE_PCR17 = PORT_PCR_MUX(3);

  //this enables no parity and 8 bit mode
  UART2_C1 = 0x00;
  // disable transmitter and receiver while we adjust and assign values
  UART2_C2 &= UART_C2_TE_MASK;
  UART2_C2 &= UART_C2_RE_MASK;

  uint16_t SBR;
  int8_t brfa;

  SBR = moduleClk / (16 * baudRate);

  brfa = ((moduleClk *2) / (baudRate)) % 32;

  UART2_C4 |= UART_C4_BRFA(brfa);

  UART2_BDH |= 0x1F & (SBR >>8);

  UART2_BDL = (uint8_t) SBR;

  //Enable transmitter and receiver
  UART2_C2 |= UART_C2_TE_MASK;
  UART2_C2 |= UART_C2_RE_MASK;

  //initialise transmit and receive FIFO
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);

  return TRUE;
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
  if (UART2_S1 & UART_S1_RDRF_MASK)
    FIFO_Put(&RxFIFO, UART2_D);

  if (UART2_S1 & UART_S1_TDRE_MASK)
    FIFO_Get(&TxFIFO, &UART2_D);
}






