/*! @file
 *
 *  @brief Routines to implement packet encoding and decoding for the serial port.
 *
 *  This contains the implementations of the functions for implementing the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-09
 */


#include "packet.h"
#include "UART.h"

uint8_t 	Packet_Command,		/*!< The packet's command */
		Packet_Parameter1, 	/*!< The packet's 1st parameter */
		Packet_Parameter2, 	/*!< The packet's 2nd parameter */
		Packet_Parameter3,	/*!< The packet's 3rd parameter */
		Packet_Checksum;	/*!< The packet's checksum */


const uint8_t PACKET_ACK_MASK = 0x80; // 1000 0000

/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *  @return bool - TRUE if the packet module was successfully initialized.
 */
bool Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //initialising UART
  return UART_Init(baudRate, moduleClk);
}







/*! @brief Attempts to get a packet from the received data.
 *
 *  @return bool - TRUE if a valid packet was received.
 */
bool Packet_Get(void)
{
  static uint8_t state;

  for (;;)
  {
    switch(state)
    {
      case 0:
	if (UART_InChar(&Packet_Command))
	  state++;

         else
	    return FALSE;
	  break;

       case 1:
	 if (UART_InChar(&Packet_Parameter1))
	     state++;

	   else
	     return FALSE;
	   break;

       case 2:
	 if (UART_InChar(&Packet_Parameter2))
	     state++;

	   else
	     return FALSE;
	   break;

       case 3:
	 if (UART_InChar(&Packet_Parameter3))
	     state++;
	   else
	     return FALSE;
	   break;

       case 4:
	 if (!UART_InChar(&Packet_Checksum))
	   return FALSE;

       case 5:
	   if(Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3 == Packet_Checksum)
	     {
	       state = 0;
	       return TRUE;
	     }

	   else
	     {
	       Packet_Parameter1 = Packet_Parameter2;
	       Packet_Parameter2 = Packet_Parameter3;
	       Packet_Parameter3 = Packet_Checksum;
	       state = 4;
	     }
	     break;
     }

    }
}







/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *
 *  @return bool - TRUE if a valid packet was sent.
 */
bool Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{

  return UART_OutChar(command) &&
	 UART_OutChar(parameter1) &&
	 UART_OutChar(parameter2) &&
	 UART_OutChar(parameter3) &&
	 UART_OutChar(command ^ parameter1 ^ parameter2 ^ parameter3);


}

