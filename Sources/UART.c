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
#include "MK70F12.h"

#define SAMPLERATE 16

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

  UART2_S2 |= UART_S2_RAF_MASK;

  uint16_t BRFD;
  int16_t brfa;

  BRFD = moduleClk % (SAMPLERATE * baudRate);

  brfa = BRFD * 32;


  UART2_C4 |= UART_C4_BRFA(brfa);
}

/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return bool - TRUE if the receive FIFO returned a character.
 *  @note Assumes that UART_Init has been called.
 */
bool UART_InChar(uint8_t* const dataPtr)
{

}

/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return bool - TRUE if the data was placed in the transmit FIFO.
 *  @note Assumes that UART_Init has been called.
 */
bool UART_OutChar(const uint8_t data)
{

}

/*! @brief Poll the UART status register to try and receive and/or transmit one character.
 *
 *  @return void
 *  @note Assumes that UART_Init has been called.
 */
void UART_Poll(void)
{

}






