/*! @file
 *
 *  @brief Routines to access the LEDs on the TWR-K70F120M.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2015-08-15
 */

// new types
#include "LEDs.h"

/*! @brief LED to pin mapping on the TWR-K70F120M
 *
 */


/*! @brief Sets up the LEDs before first use.
 *
 *  @return bool - TRUE if the LEDs were successfully initialized.
 */
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
  GPIOA_PDOR &= GPIO_PDOR_PDO_SHIFT;
  GPIOA_PSOR &= GPIO_PSOR_PTSO_SHIFT;
  GPIOA_PCOR &= GPIO_PCOR_PTCO_SHIFT;
  GPIOA_PTOR &= GPIO_PTOR_PTTO_SHIFT;
  GPIOA_PDDR &= GPIO_PDDR_PDD_SHIFT;

  return 1;
}

/*! @brief Turns an LED on.
 *
 *  @param color The color of the LED to turn on.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_On(const TLED color)
{
  GPIOA_PDDR |= GPIO_PSOR_PTSO(color);
}

/*! @brief Turns off an LED.
 *
 *  @param color THe color of the LED to turn off.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Off(const TLED color)
{
  GPIOA_PDDR &= ~GPIO_PCOR_PTCO(color);
}

/*! @brief Toggles an LED.
 *
 *  @param color THe color of the LED to toggle.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Toggle(const TLED color)
{
  GPIOA_PTOR |= GPIO_PTOR_PTTO(color);
}



