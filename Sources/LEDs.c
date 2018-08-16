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
typedef enum
{
  LED_ORANGE = (1 << 11),
  LED_YELLOW = (1 << 28),
  LED_GREEN = (1 << 29),
  LED_BLUE = (1 << 10)
} TLED;

/*! @brief Sets up the LEDs before first use.
 *
 *  @return bool - TRUE if the LEDs were successfully initialized.
 */
bool LEDs_Init(void)
{
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
  PORTA_PCR11 |= PORT_PCR_DSE_MASK;
  PORTA_PCR11 |= PORT_PCR_MUX(1);
  PORTA_PCR28 |= PORT_PCR_DSE_MASK;
  PORTA_PCR28 |= PORT_PCR_MUX(1);
  PORTA_PCR29 |= PORT_PCR_DSE_MASK;
  PORTA_PCR29 |= PORT_PCR_MUX(1);
  PORTA_PCR10 |= PORT_PCR_DSE_MASK;
  PORTA_PCR10 |= PORT_PCR_MUX(1);

  GPIOA_PDOR
  GPIOA_PSOR
  GPIOA_PCOR

  return ;
}

/*! @brief Turns an LED on.
 *
 *  @param color The color of the LED to turn on.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_On(const TLED color)
{

}

/*! @brief Turns off an LED.
 *
 *  @param color THe color of the LED to turn off.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Off(const TLED color)
{

}

/*! @brief Toggles an LED.
 *
 *  @param color THe color of the LED to toggle.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Toggle(const TLED color)
{

}

#endif
