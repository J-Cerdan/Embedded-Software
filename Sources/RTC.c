/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the real time clock (RTC).
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-09-06
 */

// new types
#include "RTC.h"
#include "MK70F12.h"
#include "PE_Types.h"



//pointer and arguments to user call back function
void (*CallBack)(void*);
void* CallBackArgument;


/*! @brief Initializes the RTC before first use.
 *
 *  Sets up the control register for the RTC and locks it.
 *  Enables the RTC and sets an interrupt every second.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the RTC was successfully initialized.
 */
bool RTC_Init(void (*userFunction)(void*), void* userArguments)
{
  //enables the RTC module
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  //Checks if the oscillator is on if not will turn it on
  if (!(RTC_CR & RTC_CR_OSCE_MASK))
    {
      RTC_CR |= RTC_CR_SC2P_MASK;
      RTC_CR |= RTC_CR_SC16P_MASK;
      //turns on the oscillator
      RTC_CR |= RTC_CR_OSCE_MASK;

      //wait for the oscillator to become stable
      for (uint32_t i=0; i<=4200000; i++)
      	{/*wait*/}
    }


  RTC_LR |= RTC_LR_CRL_MASK;
  if ((RTC_LR & RTC_LR_CRL_MASK /*&& RTC_LR & RTC_LR_SRL_MASK*/))
    {
      //use RTC_SR TCE to enable the counter
      //turns off the invalid time flag
      RTC_SR &= ~RTC_SR_TIF_MASK;
      RTC_SR &= ~RTC_SR_TOF_MASK;
      RTC_SR |= RTC_SR_TCE_MASK;


      RTC_LR |= RTC_LR_CRL_MASK;
    }

  //LR_CRL and SRL need to be locked

  NVICICER2 |= 0x04;
  RTC_IER |= RTC_IER_TSIE_MASK;
  CallBack = userFunction;
  CallBackArgument = userArguments;
  uint32_t counterTime = RTC_TCR;
  return TRUE;
}

/*! @brief Sets the value of the real time clock.
 *
 *  @param hours The desired value of the real time clock hours (0-23).
 *  @param minutes The desired value of the real time clock minutes (0-59).
 *  @param seconds The desired value of the real time clock seconds (0-59).
 *  @note Assumes that the RTC module has been initialized and all input parameters are in range.
 */
void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
  //disbale SR[TCE] before writing
  //Clear the prescaler register before writing to the seconds register.
}

/*! @brief Gets the value of the real time clock.
 *
 *  @param hours The address of a variable to store the real time clock hours.
 *  @param minutes The address of a variable to store the real time clock minutes.
 *  @param seconds The address of a variable to store the real time clock seconds.
 *  @note Assumes that the RTC module has been initialized.
 */
void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  //read the RTC_TSR
  uint32_t counterTime = RTC_TCR;
  if (counterTime != RTC_TCR)
    {
      counterTime = RTC_TSR;
    }
  *seconds = counterTime % 60;
  counterTime /= 60;

  *minutes = counterTime % 60;
  counterTime /= 60;

  *hours = counterTime % 24;

}

/*! @brief Interrupt service routine for the RTC.
 *
 *  The RTC has incremented one second.
 *  The user callback function will be called.
 *  @note Assumes the RTC has been initialized.
 */
void __attribute__ ((interrupt)) RTC_ISR(void)
{
  // SR[TOF] or SR[TIF] must both be disabled for the counter to increment
  // when SR[TOF] and SR[TIF] are set, the counter will read 0
  if (CallBack)
    (*CallBack)(CallBackArgument);
}


