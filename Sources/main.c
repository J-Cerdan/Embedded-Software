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
#include "FIFO.h"
/*#define PACKET_SPECIAL 0x04;
#define PACKET_VERSION 0x09;*/

/*UART2 on
SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
//PORTE on
SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
//Set portE bit 16 to be Alt 3 (UART2_TX function)
PORTE_PCR16 = PORT_PCR_MUX(3);
*/
/*void HandlePacket()
{
  bool success;
  switch (Packet_Command)
  {
    case PACKET_SPECIAL:
      success = HandleSpecialPacket();
      //do something in response to command 4
      break;
    case PACKET_VERSION:
      success = HandleVersionPacket();
      //do something in response to command 9
      break;
  }
  //TODO acknowledgment of packets
}*/

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */


  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  //TowerInit();
  for (;;)
  {
      /*UART_Poll();
      if (Packet_Get())
	{
	  HandlePacket();
	}*/
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



