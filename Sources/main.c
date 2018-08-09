/* ###################################################################
**     Filename    : main.c
**     Project     : Lab1
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
** @version 2.0
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
#include "packet.h"
#include "types.h"
#include "PE_Types.h"

#define PACKET_SPECIAL 0x04
#define PACKET_VERSION 0x09
#define PACKET_NUMBER 0x0B

//global private variable to store the baudRate
static const uint32_t BaudRate = 38400;
static uint16union_t TowerNumber;
static const uint8_t MajorTowerVersion = 0x01;
static const uint8_t MinorTowerVersion = 0x00;


static bool HandleNumberPacket(void)
{
  if (Packet_Parameter1 == 0x02)
    {
      TowerNumber.s.Lo = Packet_Parameter2;
      TowerNumber.s.Hi = Packet_Parameter3;
    }

  return Packet_Put(0x0B, 0x01, TowerNumber.s.Lo, TowerNumber.s.Hi);
}

static bool HandleVersionPacket(void)
{
  return Packet_Put(0x09, 0x76, MajorTowerVersion, MinorTowerVersion);
}

static bool HandleSpecialPacket(void)
{
  return Packet_Put(0x04, 0x00, 0x00, 0x00) &
	 HandleVersionPacket() &
	 HandleNumberPacket();
}

static void HandlePacket()
{
  uint8_t requiresAck;
  uint8_t success;

  if (Packet_Command & PACKET_ACK_MASK)
    {
      Packet_Command &= 0x7f;
      requiresAck = TRUE;
    }


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
   if (requiresAck)
     {
       if (success)
	 Packet_Command |= PACKET_ACK_MASK;

       Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
     }

}








/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
  TowerNumber.l = 6702;
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  Packet_Init(BaudRate, CPU_BUS_CLK_HZ);

  HandleSpecialPacket();

  for (;;)
  {
      UART_Poll();
      if (Packet_Get())
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




