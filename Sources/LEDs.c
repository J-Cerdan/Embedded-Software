/*! @file
 *
 *  @brief Routines to access the LEDs on the TWR-K70F120M.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-09-02
 */
/*!
**  @addtogroup LEDs_module packet module documentation
**  @{
*/

// new types
//includes the function prototypes to be implemented here and any public variables or constants
#include "LEDs.h"
//This header file implements peripheral memory map for MK70F1 processor.
#include "MK70F12.h"
//provides useful definitions
#include "PE_types.h"

bool LEDs_Init(void)
{
  //Turn on port A
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  //Set required portA pins to GPIO with drive strength enabled
  PORTA_PCR11 |= PORT_PCR_DSE_MASK;
  PORTA_PCR11 |= PORT_PCR_MUX(1);
  PORTA_PCR28 |= PORT_PCR_DSE_MASK;
  PORTA_PCR28 |= PORT_PCR_MUX(1);
  PORTA_PCR29 |= PORT_PCR_DSE_MASK;
  PORTA_PCR29 |= PORT_PCR_MUX(1);
  PORTA_PCR10 |= PORT_PCR_DSE_MASK;
  PORTA_PCR10 |= PORT_PCR_MUX(1);

  //Initialise GPIOA ports
  GPIOA_PDDR |= (LED_ORANGE | LED_YELLOW | LED_GREEN | LED_BLUE);
  GPIOA_PDOR |= (LED_ORANGE | LED_YELLOW | LED_GREEN | LED_BLUE);
  GPIOA_PSOR = (LED_ORANGE | LED_YELLOW | LED_GREEN | LED_BLUE);

  return TRUE;
}

void LEDs_On(const TLED color)
{
  GPIOA_PCOR = color;
}

void LEDs_Off(const TLED color)
{
  GPIOA_PSOR = color;

}

void LEDs_Toggle(const TLED color)
{
  GPIOA_PTOR = color;
}

/*!
** @}
*/



