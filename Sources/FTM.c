/*! @file
 *
 *  @brief Routines for setting up the FlexTimer module (FTM) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the FlexTimer module (FTM).
 *
 *  @author PMcL
 *  @date 2015-09-04
 */



// new types
#include "types.h"
#include "FTM.h"
#include "MK70F12.h"
#include "PE_Types.h"

#define CHANNELS 8

static void (*CallBackFunctions[CHANNELS]) (void*);
static void* CallBackArgument[CHANNELS];


/*! @brief Sets up the FTM before first use.
 *
 *  Enables the FTM as a free running 16-bit counter.
 *  @return bool - TRUE if the FTM was successfully initialized.
 */
bool FTM_Init()
{
  //Enable FTM0 clock gate control
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

  FTM0_CNTIN = ~FTM_CNTIN_INIT_MASK;

  FTM0_MOD = FTM_MOD_MOD_MASK;

  FTM0_CNT = FTM_CNT_COUNT_MASK;

  FTM0_MODE &= ~FTM_MODE_FTMEN_MASK;



  FTM0_SC = FTM_SC_CLKS(2);

  NVICISER1 |= NVIC_ICPR_CLRPEND(1 << (62 % 32));
  NVICICPR1 |= NVIC_ISER_SETENA(1 << (62 % 32));

  /*CPU_MCGFF_CLK_HZ_CONFIG_0;

  FTM0_C0SC |= FTM_CnSC_CHIE_MASK;

  FTM0_C0V = */

  return TRUE;

}

/*! @brief Sets up a timer channel.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *    channelNb is the channel number of the FTM to use.
 *    delayCount is the delay count (in module clock periods) for an output compare event.
 *    timerFunction is used to set the timer up as either an input capture or an output compare.
 *    ioType is a union that depends on the setting of the channel as input capture or output compare:
 *      outputAction is the action to take on a successful output compare.
 *      inputDetection is the type of input capture detection.
 *    callbackFunction is a pointer to a user callback function.
 *    callbackArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the timer was set up successfully.
 *  @note Assumes the FTM has been initialized.
 */
bool FTM_Set(const TFTMChannel* const aFTMChannel)
{
  EnterCritical();
  if (aFTMChannel != NULL)
    {
      if (aFTMChannel->timerFunction == TIMER_FUNCTION_INPUT_CAPTURE)
	{
	  FTM0_CnSC(aFTMChannel->channelNb) <<= (aFTMChannel->ioType.outputAction);
	  FTM0_CnSC(aFTMChannel->channelNb) &= (FTM_CnSC_MSA_MASK | FTM_CnSC_MSB_MASK);
	  CallBackFunctions[aFTMChannel->channelNb] = aFTMChannel->callbackFunction;
	  CallBackArgument[aFTMChannel->channelNb] = aFTMChannel->callbackArguments;
	  ExitCritical();
	  return TRUE;
	}
      else
	{
	  FTM0_CnSC(aFTMChannel->channelNb) <<= (aFTMChannel->ioType.inputDetection);
	  FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK;
	  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK;
	  CallBackFunctions[aFTMChannel->channelNb] = aFTMChannel->callbackFunction;
	  CallBackArgument[aFTMChannel->channelNb] = aFTMChannel->callbackArguments;
	  ExitCritical();
	  return TRUE;
	}
    }
  ExitCritical();
  return FALSE;
}


/*! @brief Starts a timer if set up for output compare.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *  @return bool - TRUE if the timer was started successfully.
 *  @note Assumes the FTM has been initialized.
 */
bool FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
  if ((aFTMChannel != NULL) && (aFTMChannel->channelNb < 8))
    {
      FTM0_CnV(aFTMChannel->channelNb) = 65536 % (FTM0_CNT + aFTMChannel->delayCount);
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK;
      FTM0_CnSC(aFTMChannel->channelNb) = FTM_CnSC_CHIE_MASK;
      return TRUE;
    }
  return FALSE;
}


/*! @brief Interrupt service routine for the FTM.
 *
 *  If a timer channel was set up as output compare, then the user callback function will be called.
 *  @note Assumes the FTM has been initialized.
 */
void __attribute__ ((interrupt)) FTM0_ISR(void)
{
  for (uint8_t i = 0; i < 8; i++)
    {
      if (FTM0_CnSC(i) & (FTM_CnSC_CHF_MASK | FTM_CnSC_CHIE_MASK))
	{
	  FTM0_CnSC(i) &= ~FTM_CnSC_CHF_MASK;
	  if (CallBackFunctions[i])
	      (*(CallBackFunctions[i]))(CallBackArgument[i]);
	}
    }
}

