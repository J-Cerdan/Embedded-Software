<<<<<<< HEAD
<<<<<<< Sources/main.c
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
//packet module - contains all the public functions to be used in this module
#include "packet.h"
//UART module - contains all the public functions to be used in this module
#include "UART.h"
//Flash module - contains all the public functions to be used in this module
#include "Flash.h"
//LED module - contains all the public functions to be used in this module
#include "LEDs.h"

//macros defined for determining which command protocol has been sent
#define PACKET_SPECIAL 0x04
#define PACKET_PROGRAM_BYTE 0x07
#define PACKET_READ_BYTE 0x08
#define PACKET_VERSION 0x09
#define PACKET_NUMBER 0x0B
#define PACKET_TOWER_MODE 0x0D

//global private constant to store the baudRate
static const uint32_t BaudRate = 115200;
//Private global variable to store the tower number
volatile uint16union_t *NvTowerNb;
volatile uint16union_t *NvTowerMd;
//Private global constants to store the major and minor tower version
static const uint8_t MajorTowerVersion = 0x01;
static const uint8_t MinorTowerVersion = 0x00;


/*! @brief Handles the "Program" request packet
 *
 *  @param None.
 *  @return bool - TRUE if the packet was written to the flash successfully
 */
static bool HandleProgramPacket(void)
{
  uint32_t address = FLASH_DATA_START;

  //Ensures incoming packet is valid
  if (Packet_Parameter1 < 0x09 && Packet_Parameter2 == 0x00)
    {
      if (Packet_Parameter1 == 0x08)
	return Flash_Erase();
      else
	return Flash_Write8((uint8_t*)(address + Packet_Parameter1), Packet_Parameter3);
    }

  return FALSE;
}

/*! @brief Handles the "Read" request packet
 *
 *  @param None.
 *  @return bool - TRUE if the packet was read from the flash successfully
 */
static bool HandleReadPacket(void)
{
  uint32_t address = FLASH_DATA_START;

  //Ensures incoming packet is valid
  if (Packet_Parameter1 < 0x08 && Packet_Parameter2 == 0x00 && Packet_Parameter3 == 0x00)
    {
      return Packet_Put(0x08, Packet_Parameter1, Packet_Parameter2, _FB(address + Packet_Parameter1));
    }

  return FALSE;
}

/*! @brief Handles the "Version number" request packet
 *
 *  @param None.
 *  @return bool - TRUE if the packet was placed in the FIFO successfully
 */
static bool HandleVersionPacket(bool specialPacket)
{
  //Ensures incoming packet is valid
  if (specialPacket == TRUE || (Packet_Parameter1 == 0x76 && Packet_Parameter2 == 0x78 && Packet_Parameter3 == 0x0D))
    return Packet_Put(0x09, 0x76, MajorTowerVersion, MinorTowerVersion);

  return FALSE;
}

/*! @brief Handles the "Tower number" request packet
 *
 *  @param No param required.
 *  @return bool - TRUE if the packet was placed in or read from the flash successfully
 */
static bool HandleNumberPacket(bool specialPacket)
{
  //if statement determines if this is a 'set' command to set a new Tower number
  if ((Packet_Parameter1 > 0x00 && Packet_Parameter1 < 0x03) || specialPacket == TRUE)
    {
      if (Packet_Parameter1 == 0x02)
	return Flash_Write16((uint16_t*)NvTowerNb, Packet_Parameter23);
      else if (!(Packet_Parameter2 || Packet_Parameter3))
	return Packet_Put(0x0B, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
    }
  return FALSE;
}

/*! @brief Handles the "Program" request packet
 *
 *  @param startUp - Identifies if the program is currently in a startUp state
 *  @return bool - TRUE if the packet was written to or read from the flash successfully
 */
static bool HandleModePacket(bool specialPacket)
{
  //if statement determines if this is a 'set' command to set a new Tower number
  if ((Packet_Parameter1 > 0x00 && Packet_Parameter1 < 0x03) || specialPacket == TRUE)
    {
      if (Packet_Parameter1 == 0x02)
        return Flash_Write16((uint16_t*)NvTowerMd, Packet_Parameter23);
      else if (!(Packet_Parameter2 || Packet_Parameter3))
	return Packet_Put(0x0D, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi);
    }

  return FALSE;
}

/*! @brief Handles the "Special" request packet
 *
 *  @param None.
 *  @return bool - TRUE if all the functions that were called were successful
 */
static bool HandleSpecialPacket(bool startUp)
{
  bool specialPacket = TRUE;
  if (startUp == TRUE)
    {
      return Packet_Put(0x04, 0x00, 0x00, 0x00) &&
	     HandleVersionPacket(specialPacket) &&
	     HandleNumberPacket(specialPacket) &&
	     HandleModePacket(specialPacket);
    }
  //calls to send all three packets to PC
  else if (!(Packet_Parameter1 || Packet_Parameter2 || Packet_Parameter3))
   {
      return Packet_Put(0x04, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3) &&
	     HandleVersionPacket(specialPacket) &&
	     HandleNumberPacket(specialPacket) &&
	     HandleModePacket(specialPacket);
   }
 return FALSE;
}

/*! @brief Handles the packets that comes from the PC and determines what to do
 *
 *  @param None.
 *  @return None.
 */
static void HandlePacket(void)
{
  uint8_t success; //used to store whether the tower executed all the requests successfully

  //handles the requests packets coming from the PC
  switch (Packet_Command & ~PACKET_ACK_MASK)
  {
    case (PACKET_SPECIAL):
	success = HandleSpecialPacket(FALSE);
    break;

    case (PACKET_PROGRAM_BYTE):
	success = HandleProgramPacket();
    break;

    case (PACKET_READ_BYTE):
	success = HandleReadPacket();
    break;

    case (PACKET_VERSION):
	success = HandleVersionPacket(FALSE);
    break;

    case (PACKET_NUMBER):
	success = HandleNumberPacket(FALSE);
    break;

    case (PACKET_TOWER_MODE):
	success = HandleModePacket(FALSE);
    break;


   break;
  }
   if (Packet_Command & PACKET_ACK_MASK) //sends acknowledgment (if PC requested it) packet to PC
     {
       if (!success) //changes the 7th bit to a 1 if tower was successful in executing the PC request
	 Packet_Command &= ~PACKET_ACK_MASK;
       //sends the acknowledgment packet to the PC
       Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
     }

}

/*! @brief Allocates space in the flash for Tower Number and Mode then writes data to the flash if required
 *
 *  @param void
 *  @return void
 */
static void TowerNumberModeInit(void)
{
  uint16_t towerNumber = 6702;
  uint16_t towerMode = 1;

  if (Flash_AllocateVar((volatile void **)&NvTowerNb, sizeof(*NvTowerNb)) &&
      Flash_AllocateVar((volatile void **)&NvTowerMd, sizeof(*NvTowerMd)))
    {
      if ((*NvTowerNb).l == 0xffff) //writes the tower number in the flash if nothing is there
	Flash_Write16((uint16_t*)NvTowerNb, towerNumber);

      if ((*NvTowerMd).l == 0xffff) //writes the tower mode in the flash if nothing is there
	Flash_Write16((uint16_t*)NvTowerMd, towerMode);
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
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/
  LEDs_Init();

  if (Packet_Init(BaudRate, CPU_BUS_CLK_HZ) && Flash_Init())
    LEDs_On(LED_ORANGE);

  //handles the initialization tower number and mode in the flash
  TowerNumberModeInit();

  //sends the initial packets when the tower starts up
  HandleSpecialPacket(TRUE);


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




=======
/* ###################################################################
**     Filename    : main.c
**     Project     : Lab1
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
// CPU module - contains low level hardware initialization routines
#include "Cpu.h"
//packet module - contains all the public functions to be used in this module
#include "packet.h"
//packet module - contains all the public functions to be used n this module
#include "UART.h"

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




>>>>>>> Sources/main.c
=======
/* ###################################################################
**     Filename    : main.c
**     Project     : Lab3
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
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
** @version 3.0
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


// CPU module - contains low level hardware initialization routines
#include "Cpu.h"

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  for (;;)
  {
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
>>>>>>> origin/Lab-3-template
