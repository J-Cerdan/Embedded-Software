/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the real time clock (RTC).
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-09-06
 */

#ifndef RTC_H
#define RTC_H

// new types
#include "RTC.h"
#include "MK70F12.h"
#include "PE_Types.h"

static uint8_t Hours = 0, Minutes = 0, Seconds = 0;

//pointer and arguments to user call back function
void (*CallBack)();
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
  RTC_CR |= RTC_CR_SC2P_MASK;
  RTC_CR |= RTC_CR_SC16P_MASK;
  RTC_CR |= RTC_CR_OSCE_MASK;

  for (int i=0; i<=500000000; i++)
    {/*wait*/}

  RTC_IER |= RTC_IER_TSIE_MASK;

  CallBack = userFunction;
  CallBackArgument = userArguments;
}

/*! @brief Sets the value of the real time clock.
 *
 *  @param hours The desired value of the real time clock hours (0-23).
 *  @param minutes The desired value of the real time clock minutes (0-59).
 *  @param seconds The desired value of the real time clock seconds (0-59).
 *  @note Assumes that the RTC module has been initialized and all input parameters are in range.
 */
void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds);

/*! @brief Gets the value of the real time clock.
 *
 *  @param hours The address of a variable to store the real time clock hours.
 *  @param minutes The address of a variable to store the real time clock minutes.
 *  @param seconds The address of a variable to store the real time clock seconds.
 *  @note Assumes that the RTC module has been initialized.
 */
void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds);

/*! @brief Interrupt service routine for the RTC.
 *
 *  The RTC has incremented one second.
 *  The user callback function will be called.
 *  @note Assumes the RTC has been initialized.
 */
void __attribute__ ((interrupt)) RTC_ISR(void)
{
  (CallBack)(CallBackArgument);
}


#endif