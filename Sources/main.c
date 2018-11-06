/* ###################################################################
**     Filename    : main.c
**     Project     : Lab4
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
** @version 6.0
** @brief
**         Main module.
**         This module contains user's application code.
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
#include "RTC.h"
#include "PIT.h"
#include "FTM.h"
#include "analog.h"
#include "OS.h"
#include "ThreadManage.h"
#include "Waveform.h"
#include "AWG.h"


//macros defined for determining which command protocol has been sent
#define PACKET_SPECIAL 0x04
#define PACKET_PROGRAM_BYTE 0x07
#define PACKET_READ_BYTE 0x08
#define PACKET_VERSION 0x09
#define PACKET_NUMBER 0x0B
#define PACKET_TOWER_MODE 0x0D
#define PACKET_SET_TIME 0x0C
#define PACKET_PROTOCOL_MODE 0x0A
#define PACKET_ANALOG_INPUT_VALUE 0x50
#define PACKET_WAVE 0x60
#define PACKET_ARB_WAVE 0x61

//macros defined for determining configurations to be executed
#define STATUS 0x00
#define WAVEFORM 0x01
#define FREQUENCY 0x02
#define AMPLITUDE 0x03
#define OFFSET 0x04
#define ALL_WAVEFORMS_ON 0x05
#define ALL_WAVEFORMS_OFF 0x06
#define ACTIVE_CHANNEL 0x07

//macros defined for arbitrary wave functions
#define ARB_CHANNEL_00 0x00
#define ARB_CHANNEL_01 0x01
#define ARB_VALUE_RECEIVE 0x02
#define ARB_VALUE_END 0xFF
#define MAX_ARB_ARRAY 1000


//global private constant to store the baudRate
static const uint32_t BaudRate = 115200;
//Private global variable to store the tower number
static volatile uint16union_t *NvTowerNb;
static volatile uint16union_t *NvTowerMd;
//Private global constants to store the major and minor tower version
static const uint8_t MajorTowerVersion = 0x01;
static const uint8_t MinorTowerVersion = 0x00;
//TFTMChannel variable for Channel 0
static TFTMChannel Ch0;
//store channel to be synchronous or asynchronous
static bool synchronous = FALSE;
//LTC1859 channel to be used
static const uint8_t ADCCHANNEL = 0;
//RTC Time
static uint8_t Hours = 0, Minutes = 0, Seconds = 0;
//Arbitrary Build Channel
static uint8_t ArbChannel = 0;


//PacketThread and InitThread stack
OS_THREAD_STACK(PacketStack, THREAD_STACK_SIZE);
OS_THREAD_STACK(InitStack, THREAD_STACK_SIZE);
OS_THREAD_STACK(PITStack, THREAD_STACK_SIZE);
OS_THREAD_STACK(Ch00Stack, THREAD_STACK_SIZE);
OS_THREAD_STACK(Ch01Stack, THREAD_STACK_SIZE);



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
    return Packet_Put(PACKET_READ_BYTE, Packet_Parameter1, Packet_Parameter2, _FB(address + Packet_Parameter1));
  }

  return FALSE;
}

/*! @brief Handles the "Version number" request packet
 *
 *  @param specialPacket - Identifies if the program is currently in a startUp state
 *  @return bool - TRUE if the packet was placed in the FIFO successfully
 */
static bool HandleVersionPacket(bool specialPacket)
{
  //Ensures incoming packet is valid
  if (specialPacket == TRUE || (Packet_Parameter1 == 0x76 && Packet_Parameter2 == 0x78 && Packet_Parameter3 == 0x0D))
    return Packet_Put(PACKET_VERSION, 0x76, MajorTowerVersion, MinorTowerVersion);

  return FALSE;
}

/*! @brief Handles the "Tower number" request packet
 *
 *  @param specialPacket - Identifies if the program is currently in a startUp state
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
      return Packet_Put(PACKET_NUMBER, 0x01, (*NvTowerNb).s.Lo, (*NvTowerNb).s.Hi);
    }
  return FALSE;
}

/*! @brief Handles the "Tower Mode" request packet
 *
 *  @param specialPacket - Identifies if the program is currently in a startUp state
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
      return Packet_Put(PACKET_TOWER_MODE, 0x01, (*NvTowerMd).s.Lo, (*NvTowerMd).s.Hi);
  }

  return FALSE;
}

/*! @brief Handles the "Set Time" request packet
 *
 */
static bool HandleTimePacket(void)
{
  if (Packet_Parameter1 < 24 && Packet_Parameter2 < 60 && Packet_Parameter3 < 60 )
  {
    RTC_Set(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    return TRUE;
  }
  return FALSE;
}

/*! @brief Handles the "Protocol - Mode" request packet
 *
 *  @param specialPacket - Identifies if the program is currently in a startUp state
 *  @return bool - TRUE if the parameters were correct and packet packet was sent to PC
 */
static bool HandleProtocolPacket(bool specialPacket)
{
  //checks if packet is valid
  if (Packet_Parameter1 == 1 && Packet_Parameter2 == 0 && Packet_Parameter3 == 0)
  {
    //sends the current protocol mode
    Packet_Put(PACKET_PROTOCOL_MODE, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    return TRUE;
  }
  //checks if packet is valid
  if ((Packet_Parameter1 = 2 && Packet_Parameter2 >= 0 && Packet_Parameter2 <= 1 && Packet_Parameter3 == 0) || specialPacket == TRUE)
  {
    if(specialPacket == FALSE && Packet_Parameter2 == 0)
      synchronous = FALSE;
    else if (specialPacket == FALSE)
      synchronous = TRUE;

    Packet_Put(PACKET_PROTOCOL_MODE, 0x01, (uint8_t)synchronous, 0x00);
    return TRUE;
  }

  return FALSE;
}


/*! @brief Handles the Waveform packet
 *
 *  @param None.
 *  @return bool - TRUE if all the functions that were called were successful
 */
static bool HandleWavePacket(void)
{
  switch (Packet_Parameter1)
  {
    case (STATUS):
      if (Packet_Parameter2 < 2 && Packet_Parameter3 < 2)
        return Set_Status(Packet_Parameter2, Packet_Parameter3);
      break;

    case (WAVEFORM):
      if (Packet_Parameter2 < 4 || Packet_Parameter2 == 5)
        return Set_Waveform(Packet_Parameter2);
      break;

    case (FREQUENCY):
      return Set_Frequency(Packet_Parameter2, Packet_Parameter3);
      break;

    case (AMPLITUDE):
      return Set_Amplitude(Packet_Parameter2, Packet_Parameter3);
      break;

    case (OFFSET):
      return Set_Offset(Packet_Parameter2, Packet_Parameter3);
      break;

    case (ALL_WAVEFORMS_ON):
      return Set_AllWaveformStatus(TRUE);
      break;

    case (ALL_WAVEFORMS_OFF):
      return Set_AllWaveformStatus(FALSE);
      break;

    case (ACTIVE_CHANNEL):
      if (Packet_Parameter2 < 2)
        return Set_Active(Packet_Parameter2);
      break;
  }

  return FALSE;

}

/*! @brief Handles the Arbitrary Wave packet
 *
 *  @param None.
 *  @return bool - TRUE if all the functions that were called were successful
 */
static bool HandleArbWavePacket(void)
{
  //Temporary variable to store received data
  int16union_t arbitraryValue;

  switch (Packet_Parameter1)
    {
      //End if reached max array size
      if (ArbIndex = MAX_ARB_ARRAY)
      {
	Packet_Parameter1 = ARB_VALUE_END;
      }

      case (ARB_CHANNEL_00):
        ArbChannel = 0;
	ArbIndex = 0;
	arbitraryValue.s.Lo = Packet_Parameter2;
	arbitraryValue.s.Hi = Packet_Parameter3;

	Arbitrary_AddValues(ArbChannel, arbitraryValue.l);
	break;

      case (ARB_CHANNEL_01):
        ArbChannel = 1;
      	ArbIndex = 0;
      	arbitraryValue.s.Lo = Packet_Parameter2;
      	arbitraryValue.s.Hi = Packet_Parameter3;

      	Arbitrary_AddValues(ArbChannel, arbitraryValue.l);
        break;

      case (ARB_VALUE_RECEIVE):
      	arbitraryValue.s.Lo = Packet_Parameter2;
      	arbitraryValue.s.Hi = Packet_Parameter3;

      	Arbitrary_AddValues(ArbChannel, arbitraryValue.l);
        break;

      case (ARB_VALUE_END):
    	DACChannel[ArbChannel].arbitrarymaxindex = ArbIndex;
	return TRUE;
        break;
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
    return Packet_Put(PACKET_SPECIAL, 0x00, 0x00, 0x00) &&
	   HandleVersionPacket(specialPacket) &&
	   HandleNumberPacket(specialPacket) &&
	   HandleModePacket(specialPacket) &&
	   HandleProtocolPacket(specialPacket);
  }
  //calls to send all three packets to PC
  else if (!(Packet_Parameter1 || Packet_Parameter2 || Packet_Parameter3))
  {
    return Packet_Put(PACKET_SPECIAL, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3) &&
	   HandleVersionPacket(specialPacket) &&
	   HandleNumberPacket(specialPacket) &&
	   HandleModePacket(specialPacket)&&
	   HandleProtocolPacket(specialPacket);
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
  LEDs_On(LED_BLUE);
  FTM_StartTimer(&Ch0);
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

    case (PACKET_SET_TIME):
      success = HandleTimePacket();
      break;

    case (PACKET_PROTOCOL_MODE):
      success = HandleProtocolPacket(FALSE);
      break;

    case (PACKET_WAVE):
      success = HandleWavePacket();
      break;

    case (PACKET_ARB_WAVE):
      success = HandleArbWavePacket();
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


/*! @brief Call back functions for the PIT ISR
 *
 *  @param void
 *  @return void* argument for the ISR
 */
static void PITCallback(void* arg)
{

  //Analog_Get(ADCCHANNEL);

}


/*! @brief Call back functions for the RTC ISR
 *
 *  @param void
 *  @return void* argument for the ISR
 */
static void RTCCallback (void* arg)
{
  RTC_Get(&Hours, &Minutes, &Seconds);
  Packet_Put(0x0C, Hours, Minutes, Seconds);
  LEDs_Toggle(LED_YELLOW);

}

/*! @brief Call back functions for the FTM) channel 0 ISR
 *
 *  @param void
 *  @return arg argument for the ISR
 */
static void FTM0CH0Callback(void* arg)
{
  LEDs_Off(LED_BLUE);

}

/*! @brief builds the TFTMChannel struct for channel 0
 *
 *  @param void
 *  @return void
 */
static void CH01SecondTimerInit(void)
{
  Ch0.channelNb = 0;
  Ch0.delayCount = CPU_MCGFF_CLK_HZ_CONFIG_0;
  Ch0.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;
  Ch0.ioType.outputAction = TIMER_OUTPUT_HIGH;
  Ch0.callbackFunction = FTM0CH0Callback;
  Ch0.callbackArguments = NULL;
  FTM_Set(&Ch0);
}

static void PITThread(void* arg)
{
  for (;;)
  {
    (void)OS_SemaphoreWait(CntDone, 0);

    //counter used to determine if 500ms has passed
    static uint8_t ledToggleCount = 0;
    ledToggleCount++;
    if (ledToggleCount == 50)
    {
      LEDs_Toggle(LED_GREEN);
      ledToggleCount = 0;
    }
    /*
    if(synchronous)
    {
      Packet_Put(PACKET_ANALOG_INPUT_VALUE, 0x00, Analog_Input[ADCCHANNEL].value.s.Lo, Analog_Input[ADCCHANNEL].value.s.Hi);
    }
    else
    {
      if (Analog_Input[ADCCHANNEL].value.l != Analog_Input[ADCCHANNEL].oldValue.l)
	Packet_Put(PACKET_ANALOG_INPUT_VALUE, 0x00, Analog_Input[ADCCHANNEL].value.s.Lo, Analog_Input[ADCCHANNEL].value.s.Hi);
    }
    */
  }
}

/*! @brief Thread to handle packets received
 *
 *  @param void* arg
 *  @return void
 */
static void PacketThread(void* arg)
{
  for (;;)
  {
    Packet_Get();
    HandlePacket();

  }
}

/*! @brief Thread to handle all the initialisation
 *
 *  @param void* arg
 *  @return void
 */
static void InitThread(void* arg)
{
  for (;;)
  {
    OS_DisableInterrupts();
    LEDs_Init();

    if (Packet_Init(BaudRate, CPU_BUS_CLK_HZ) && Flash_Init() && Analog_Init(CPU_BUS_CLK_HZ)
	&&  RTC_Init(RTCCallback, NULL) && PIT_Init(CPU_BUS_CLK_HZ, PITCallback, NULL) &&
	FTM_Init() && AWG_Init())
      LEDs_On(LED_ORANGE);

    //setup the PIT and call for Channel 0 to be set up
    PIT_Set(500000, TRUE);
    CH01SecondTimerInit();

    //handles the initialization tower number and mode in the flash
    TowerNumberModeInit();

    //sends the initial packets when the tower starts up
    HandleSpecialPacket(TRUE);

    OS_EnableInterrupts(); //enable interrupts

    OS_ThreadDelete(OS_PRIORITY_SELF);
  }

}

/*! @brief Thread to handle processing for Channel 1 Thread
 *
 *  @param void* arg
 *  @return void
 */
static void Ch00Thread(void* arg)
{
  for (;;)
  {
    //check if channel processing is active
    (void)OS_SemaphoreWait(Ch00Processing, 0);

    //Obtain value for channel 0
    Ch00_Value = AWG_DAC_Get(DAC_CHANNEL_00);

    if (Ch00_Value >= 65535)
    {
	Ch00_Value = 0xFFFF;
    }
    else if (Ch00_Value <= 0)
    {
	Ch00_Value = 0;
    }

  }
}

/*! @brief Thread to handle processing for Channel 1 Thread
 *
 *  @param void* arg
 *  @return void
 */
static void Ch01Thread(void* arg)
{
  for (;;)
  {
    //check if channel processing is active
    (void)OS_SemaphoreWait(Ch01Processing, 0);

    //Obtain value for channel 1
    Ch01_Value = AWG_DAC_Get(DAC_CHANNEL_01);

    if (Ch01_Value >= 65535)
    {
      Ch01_Value = 0xFFFF;
    }
    else if (Ch01_Value <= 0)
    {
      Ch01_Value = 0;
    }

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

  OS_Init(CPU_CORE_CLK_HZ, false);

  OS_ThreadCreate(InitThread, NULL, &InitStack[THREAD_STACK_SIZE - 1], INIT_THREAD);

  OS_ThreadCreate(PacketThread, NULL, &PacketStack[THREAD_STACK_SIZE - 1], PACKET_THREAD);

  OS_ThreadCreate(PITThread, NULL, &PITStack[THREAD_STACK_SIZE - 1], PIT_THREAD);

  OS_ThreadCreate(Ch00Thread, NULL, &Ch00Stack[THREAD_STACK_SIZE - 1], CH00_THREAD);

  OS_ThreadCreate(Ch01Thread, NULL, &Ch01Stack[THREAD_STACK_SIZE - 1], CH01_THREAD);


  //begin multithreading
  OS_Start();


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
