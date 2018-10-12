/*! @file
 *
 *  @brief Routines for setting up the FlexTimer module (FTM) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the FlexTimer module (FTM).
 *
 *  @author PMcL
 *  @date 2015-09-04
 */

/*!
**  @addtogroup FTM_module FTM module documentation
**  @{
*/

// new types
#include "types.h"
#include "FTM.h"
#include "MK70F12.h"
#include "PE_Types.h"
#include "OS.h"
#include "ThreadManage.h"

#define CHANNELS 8

//arrays to store the call back functions and arguments for each channel of the FTM0
static void (*CallBackFunctions[CHANNELS]) (void*);
static void* CallBackArgument[CHANNELS];

//
static uint8_t ChannelInterrupt;
//stack for thread
OS_THREAD_STACK(FTMStack, THREAD_STACK_SIZE);
//semaphore
static OS_ECB* CntDone;

static void FTMThread(void* arg);

bool FTM_Init()
{
  //Enable FTM0 clock gate control
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

  //setting the counter to be free running
  FTM0_CNTIN = ~FTM_CNTIN_INIT_MASK;
  FTM0_MOD = FTM_MOD_MOD_MASK;
  FTM0_CNT = ~FTM_CNT_COUNT_MASK;
  FTM0_SC |= FTM_SC_CLKS(2);
  FTM0_MODE |= FTM_MODE_FTMEN_MASK;

  //enabling the NVIC registers
  NVICISER1 |= NVIC_ISER_SETENA(1 << (62 % 32));
  NVICICPR1 |= NVIC_ICPR_CLRPEND(1 << (62 % 32));

  //create the thread
  OS_ThreadCreate(FTMThread, NULL, &FTMStack[THREAD_STACK_SIZE - 1], FTM_THREAD);

  //create semaphore
  CntDone = OS_SemaphoreCreate(0);

  return TRUE;

}


bool FTM_Set(const TFTMChannel* const aFTMChannel)
{
  OS_DisableInterrupts(); //critical section as global variables are being adjusted
  if (aFTMChannel != NULL) //make sure pointer is not NULL
    {
      if (aFTMChannel->timerFunction == TIMER_FUNCTION_INPUT_CAPTURE)//sets up channel for input capture
	{
	  FTM0_CnSC(aFTMChannel->channelNb) = ((aFTMChannel->ioType.outputAction) << 2);
	  FTM0_CnSC(aFTMChannel->channelNb) &= (FTM_CnSC_MSA_MASK | FTM_CnSC_MSB_MASK);
	  CallBackFunctions[aFTMChannel->channelNb] = aFTMChannel->callbackFunction;
	  CallBackArgument[aFTMChannel->channelNb] = aFTMChannel->callbackArguments;
	  OS_EnableInterrupts();
	  return TRUE;
	}
      else //sets up channel for output compare
	{
	  FTM0_CnSC(aFTMChannel->channelNb) = ((aFTMChannel->ioType.inputDetection) << 2);
	  FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK;
	  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK;
	  CallBackFunctions[aFTMChannel->channelNb] = aFTMChannel->callbackFunction;
	  CallBackArgument[aFTMChannel->channelNb] = aFTMChannel->callbackArguments;
	  OS_EnableInterrupts();
	  return TRUE;
	}
    }
  OS_EnableInterrupts();
  return FALSE;
}



bool FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
  if ((aFTMChannel != NULL) && (aFTMChannel->channelNb < 8)) //makes sure pointer and channel is valid
    {

      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHIE_MASK; //disable interrupts while timer is being set
      FTM0_CnV(aFTMChannel->channelNb) = (FTM0_CNT + aFTMChannel->delayCount); // set the timer value

      //clear the flag and enable the interrupt bit
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK;
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK;
      return TRUE;
    }
  return FALSE;
}



void __attribute__ ((interrupt)) FTM0_ISR(void)
{
  OS_ISREnter();
  //loops through the channels to see which generated the interrupt and call is callback function
  for (uint8_t i = 0; i < CHANNELS; i++)
    {
      if (FTM0_CnSC(i) & (FTM_CnSC_CHF_MASK | FTM_CnSC_CHIE_MASK))
	{
	  FTM0_CnSC(i) &= ~(FTM_CnSC_CHF_MASK | FTM_CnSC_CHIE_MASK);
	  ChannelInterrupt |= (1 << i); //indicates which channel needs to be serviced
	  (void)OS_SemaphoreSignal(CntDone);
	}
    }
  OS_ISRExit();
}

static void FTMThread(void* arg)
{
  for (;;)
    {
      (void)OS_SemaphoreWait(CntDone, 0);
      for(uint8_t i = 0; i < CHANNELS; i++) //loop checks which channel needs servicing and call its call back function
	{
	  if(ChannelInterrupt & (1 << i))
	    {
	      if (CallBackFunctions[i])
		(*(CallBackFunctions[i]))(CallBackArgument[i]);
	      ChannelInterrupt &= ~(1 << i); //indicates that channel has been serviced
	      break;
	    }
	}
    }
}

/*!
** @}
*/

