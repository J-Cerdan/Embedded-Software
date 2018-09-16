/*! @file
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT) on the TWR-K70F120M.
 *
 *  This contains the functions for operating the periodic interrupt timer (PIT).
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-09-14
 */
/*!
**  @addtogroup PIT_module PIT module documentation
**  @{
*/

// new types
//includes the function prototypes to be implemented here and any public variables or constants
#include "PIT.h"
//LED module - contains all the public functions to be used in this module
#include "LEDs.h"
//provides useful definitions
#include "PE_Types.h"
//This header file implements peripheral memory map for MK70F1 processor.
#include "MK70F12.h"

//Private global variable to contain module clock
static uint32_t ModuleClk;

//pointer and arguments to user call back function
static void (*CallBack)(void*);
static void* CallBackArgument;


/*! @brief Sets up the PIT before first use.
 *
 *  Enables the PIT and freezes the timer when debugging.
 *  @param moduleClk The module clock rate in Hz.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the PIT was successfully initialized.
 *  @note Assumes that moduleClk has a period which can be expressed as an integral number of nanoseconds.
 */
bool PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{

  ModuleClk = moduleClk;

  //Enable PIT clock gate control
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

  //W1C before interrupt enable
  PIT_TFLG0 = PIT_TFLG_TIF_MASK;

  //PIT Timer Interrupt Enable
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;

  PIT_Enable(TRUE);


  NVICISER2 |= NVIC_ISER_SETENA(1 << (68 % 32));
  NVICICPR2 |= NVIC_ICPR_CLRPEND(1 << (68 % 32));


  CallBack = userFunction;
  CallBackArgument = userArguments;

  return TRUE;

}

/*! @brief Sets the value of the desired period of the PIT.
 *
 *  @param period The desired value of the timer period in nanoseconds.
 *  @param restart TRUE if the PIT is disabled, a new value set, and then enabled.
 *                 FALSE if the PIT will use the new value after a trigger event.
 *  @note The function will enable the timer and interrupts for the PIT.
 */
void PIT_Set(const uint32_t period, const bool restart)
{
  //Critical mode to stop foreground or background operations that could affect the set process
  EnterCritical();

  uint32_t clockPeriod;

  //Calculation to determine clock cycles needed
  clockPeriod = 1000000000/ModuleClk;

  //Set timer start value
  PIT_LDVAL0 = ((period/clockPeriod) - 1);

  // If restart is true, disable then enable timer to restart
  if (restart)
    {
      PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
      PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
    }

  ExitCritical();

}

/*! @brief Enables or disables the PIT.
 *
 *  @param enable - TRUE if the PIT is to be enabled, FALSE if the PIT is to be disabled.
 */
void PIT_Enable(const bool enable)
{
  if (enable)
    // Enable PIT Module at MCR = 0 (PIT timers enabled)
    PIT_MCR &= ~PIT_MCR_MDIS_MASK;

  else
    // Disable PIT Module at MCR = 1 (PIT timers disabled)
    PIT_MCR |= PIT_MCR_MDIS_MASK;

}

/*! @brief Interrupt service routine for the PIT.
 *
 *  The periodic interrupt timer has timed out.
 *  The user callback function will be called.
 *  @note Assumes the PIT has been initialized.
 */
void __attribute__ ((interrupt)) PIT_ISR(void)
{
  //Write 1 to clear interrupt flag
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

  if (CallBack)
  (*CallBack) (CallBackArgument);

}

/*!
** @}
*/
