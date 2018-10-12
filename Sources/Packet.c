/*! @file
 *
 *  @brief Routines to implement packet encoding and decoding for the serial port.
 *
 *  This contains the functions for implementing the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-08-10
 */
/*!
**  @addtogroup packet_module packet module documentation
**  @{
*/
//provides useful definitions
#include "PE_Types.h"
//includes the function prototypes to be implemented here and any public variables or constants
#include "packet.h"
//provides useful public functions
#include "UART.h"
#include "OS.h"


const uint8_t PACKET_ACK_MASK = 0x80; // 1000 0000

TPacket Packet;


/*! @brief Calculates the checksum of the packet through XORing the parameters passed in.
 *
 *  @param command Command byte of packet.
 *  @param parameter1 Parameter 1 byte of packet.
 *  @param parameter2 Parameter 2 byte of packet.
 *  @param parameter3 Parameter 3 byte of packet.
 *  @return uint8_t - Checksum calculation after XOR.
 */
static uint8_t CalculateChecksum(uint8_t command, uint8_t parameter1, uint8_t parameter2, uint8_t parameter3);

bool Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //Calls and initiates UART_Init in order to ensure that packets are initialised
  return UART_Init(baudRate, moduleClk);
}


bool Packet_Get(void)
{
  //Initialisation of state variable for the switch statement, is static to maintain position of previous state
  static uint8_t state;

  //Loop implemented to handle and store packets from data
  for (;;)
  {
    switch(state)
    {
      //Store into command packet and check if storage has been completed
      case 0:
    if (UART_InChar(&Packet_Command))
      state++;

        else
      return FALSE;
    break;

      //Store into first parameter packet and check if storage has been completed
      case 1:
    if (UART_InChar(&Packet_Parameter1))
        state++;

    else
      return FALSE;
    break;

      //Store into second parameter packet and check if storage has been completed
      case 2:
    if (UART_InChar(&Packet_Parameter2))
        state++;

    else
       return FALSE;
    break;

      //Store into third parameter packet and check if storage has been completed
      case 3:
    if (UART_InChar(&Packet_Parameter3))
       state++;

    else
       return FALSE;
    break;

      //Store into checksum packet and check if storage has been completed
      case 4:
    if (!UART_InChar(&Packet_Checksum))
      return FALSE;

      //Check if Checksum is equal to the XOR of all preceding packets for synchronization
      case 5:
    if(CalculateChecksum(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3) == Packet_Checksum)
        {
          //Reinitalise state variable when packets are synced and ready to be processed by the tower
          state = 0;
          return TRUE;
        }

    //Shifts all packets one byte to read in a new checksum (for packet synchronization
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


bool Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  //Critical mode to stop foreground or background operations
  OS_DisableInterrupts();
  bool success = FALSE;
  //Obtains packets and assigns to parameters of FIFO buffer, returns 0 if any execution fails
  success = (UART_OutChar(command) &&
     UART_OutChar(parameter1) &&
     UART_OutChar(parameter2) &&
     UART_OutChar(parameter3) &&
     UART_OutChar(CalculateChecksum(command, parameter1, parameter2, parameter3))); //Calculates and stores checksum

  OS_EnableInterrupts();
  return success;
}

static uint8_t CalculateChecksum(uint8_t command, uint8_t parameter1, uint8_t parameter2, uint8_t parameter3)
{
  return (command ^ parameter1 ^ parameter2 ^ parameter3);
}




/*!
** @}
*/
