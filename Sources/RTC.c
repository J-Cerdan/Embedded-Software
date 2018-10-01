/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the real time clock (RTC).
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-09-06
 */
/*!
**  @addtogroup RTC_module RTC module documentation
**  @{
*/

// new types
#include "RTC.h"
#include "MK70F12.h"
#include "PE_Types.h"



//pointer and arguments to user call back function
static void (*CallBack)(void*);
static void* CallBackArgument;



bool RTC_Init(void (*userFunction)(void*), void* userArguments)
{
  //Critical mode to stop foreground or background operations
  EnterCritical();
  //enables the RTC module
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  // reset and see if it works. pull it out of reset if it did reset
  RTC_CR |= RTC_CR_SWR_MASK;
  if (RTC_CR & RTC_CR_SWR_MASK)
    {
      //Ensure software reset is off
      RTC_CR &= ~RTC_CR_SWR_MASK;
      //Set oscillator load
      RTC_CR |= (RTC_CR_SC2P_MASK | RTC_CR_SC16P_MASK);
      //turns on the oscillator
      RTC_CR |= RTC_CR_OSCE_MASK;

      //wait for the oscillator to become stable
      for (uint32_t i=0; i<=4200000; i++)
      	{/*wait*/}

      //lock the registers
      RTC_LR &= ~RTC_LR_CRL_MASK;

      //Disable counter with invalid flag
      if (RTC_SR & RTC_SR_TIF_MASK)
          {
            RTC_SR &= ~RTC_SR_TCE_MASK;
            RTC_TSR = 0x00;
          }
    }

  //set the NVIC registers
  NVICISER2 |= NVIC_ISER_SETENA(1 << (67 % 32));
  NVICICPR2 |= NVIC_ICPR_CLRPEND(1 << (67 % 32));


  //Enable counter and time seconds interrupt
  RTC_SR |= RTC_SR_TCE_MASK;
  RTC_IER |= RTC_IER_TSIE_MASK;

  //set user callback functions
  CallBack = userFunction;
  CallBackArgument = userArguments;
  ExitCritical();

  return TRUE;
}


void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
  if (hours < 24 && minutes < 60 && seconds < 60 )//checks time is valid
    {
      uint32_t counterTime = (hours * 3600) + (minutes * 60) + seconds;

      //Disable counter, set prescaler and seconds register, then enable
      RTC_SR &= ~RTC_SR_TCE_MASK;
      RTC_TPR = 0x00;
      RTC_TSR = counterTime;
      RTC_SR |= RTC_SR_TCE_MASK;

    }
}


void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  //read the RTC_TSR
  uint32_t counterTime = RTC_TSR;

  //check if time is the same after reading twice to make sure valid time is read
  if (counterTime != RTC_TSR)
    {
      counterTime = RTC_TSR;
    }
  *seconds = counterTime % 60;
  counterTime /= 60;

  *minutes = counterTime % 60;
  counterTime /= 60;

  *hours = counterTime % 24;

}


void __attribute__ ((interrupt)) RTC_ISR(void)
{

  if (CallBack)
    (*CallBack)(CallBackArgument);
}

/*!
** @}
*/



