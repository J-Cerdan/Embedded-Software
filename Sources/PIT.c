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
#include "OS.h"
#include "ThreadManage.h"

//Private global variable to contain module clock
static uint32_t ModuleClk;

//pointer and arguments to user call back function
static void (*CallBack)(void*);
static void* CallBackArgument;


//semaphore for PITThread()
OS_ECB* CntDone;


static void PITThread(void* arg);


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

  //create semaphore
  CntDone = OS_SemaphoreCreate(0);

  return TRUE;

}


void PIT_Set(const uint32_t period, const bool restart)
{
  //Critical mode to stop foreground or background operations that could affect the set process
  //OS_DisableInterrupts();

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

  //OS_EnableInterrupts();

}


void PIT_Enable(const bool enable)
{
  if (enable)
    // Enable PIT Module at MCR = 0 (PIT timers enabled)
    PIT_MCR &= ~PIT_MCR_MDIS_MASK;

  else
    // Disable PIT Module at MCR = 1 (PIT timers disabled)
    PIT_MCR |= PIT_MCR_MDIS_MASK;

}


void __attribute__ ((interrupt)) PIT_ISR(void)
{
  OS_ISREnter();
  //Write 1 to clear interrupt flag
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;

  if (CallBack)
    (*CallBack) (CallBackArgument); //calls the user call back function

  //signal the semaphore
  (void)OS_SemaphoreSignal(CntDone);

  OS_ISRExit();

}


/*!
** @}
*/
