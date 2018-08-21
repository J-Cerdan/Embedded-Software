/* ###################################################################
**     Filename    : main.c
**     Project     : Lab2
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2018-08-10, 13:27, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 2.0
** @brief
**         Main module.
**         This module contains user's application code.
** @date 2018-08-10
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */

//types header - provides variable types and definitions useful for pc to tower protocols
#include "types.h"
//PE_Types header - provides useful definitions
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
// CPU module - contains low level hardware initialization routines
#include "Cpu.h"
#include "Events.h"
//#include "Events.h"
//packet module - contains all the public functions to be used in this module
#include "packet.h"
//packet module - contains all the public functions to be used n this module
#include "UART.h"

#include "LEDs.h"

//macros defined for determining which command protocol has been sent
#define PACKET_SPECIAL 0x04
#define PACKET_VERSION 0x09
#define PACKET_NUMBER 0x0B

//global private constant to store the baudRate
static const uint32_t BaudRate = 38400;
//Private global variable to store the tower number
static uint16union_t TowerNumber;
//Private global constants to store the major and minor tower version
static const uint8_t MajorTowerVersion = 0x01;
static const uint8_t MinorTowerVersion = 0x00;

/*! @brief Handles the "Tower number" request packet
 *
 *  @param No param required.
 *  @return bool - TRUE if the packet was placed in the FIFO successfully
 */
static bool HandleNumberPacket(void)
{
  //if statement determines if this is a 'set' command to set a new Tower number
  if (Packet_Parameter1 == 0x02)
    {
      TowerNumber.s.Lo = Packet_Parameter2;
      TowerNumber.s.Hi = Packet_Parameter3;
    }

  return Packet_Put(0x0B, 0x01, TowerNumber.s.Lo, TowerNumber.s.Hi);
}

/*! @brief Handles the "Version number" request packet
 *
 *  @param None.
 *  @return bool - TRUE if the packet was placed in the FIFO successfully
 */
static bool HandleVersionPacket(void)
{
  return Packet_Put(0x09, 0x76, MajorTowerVersion, MinorTowerVersion);
}

/*! @brief Handles the "Special" request packet
 *
 *  @param None.
 *  @return bool - TRUE if all the functions that were called were successful
 */
static bool HandleSpecialPacket(void)
{
  //calls to send all three packets to PC
  return Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3) &
	 HandleVersionPacket() &
	 HandleNumberPacket();
}

/*! @brief Handles the packets that comes from the PC and determines what to do
 *
 *  @param None.
 *  @return None.
 */
static void HandlePacket(void)
{
  uint8_t requiresAck; //used to store whether the PC wants acknowledgment or not
  uint8_t success; //used to store whether the tower executed the all the requests successfully

  //determines if the PC wants acknowledgment
  if (Packet_Command & PACKET_ACK_MASK)
    {
      //removes acknowledgment bit from incoming command packet
      Packet_Command &= ~PACKET_ACK_MASK;
      requiresAck = TRUE;
    }

  //handles the requests packets coming from the PC
  switch (Packet_Command)
  {
    case (PACKET_SPECIAL):
     success = HandleSpecialPacket();
      break;

    case (PACKET_VERSION):
	success = HandleVersionPacket();
      break;

    case (PACKET_NUMBER):
	success = HandleNumberPacket();


   break;
  }
   if (requiresAck) //sends acknowledgment (if PC requested it) packet to PC
     {
       if (success) //changes the 7th bit to a 1 if tower was successful in executing the PC request
	 Packet_Command |= PACKET_ACK_MASK;
       //sends the acknowledgment packet to the PC
       Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
     }

}



/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
/*! @brief main
 *
 *  @return int.
 */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  // stores the tower number as a union to be able to access hi and lo bytes
  TowerNumber.l = 6702;
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/
  //initializes the Packet and passes in the baud rate and CPU bus clock
  Packet_Init(BaudRate, CPU_BUS_CLK_HZ);
  //sends the initial first three packets when the tower starts up
  HandleSpecialPacket();

  LEDs_Init();

  LEDs_On(LED_ORANGE);

  LEDs_Toggle(LED_ORANGE);


  for (;;)
  {
      UART_Poll(); //loop polling the UART to receive and sends bytes
      if (Packet_Get()) //checks if any complete packets have been received and calls the HandlePacket function
	{
	  HandlePacket();
	}
  }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/




