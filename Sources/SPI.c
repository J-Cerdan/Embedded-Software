/*! @file
 *
 *  @brief I/O routines for K70 SPI interface.
 *
 *  This contains the functions for operating the SPI (serial peripheral interface) module.
 *
 *  @author Amir Hussein & Joseph Cerdan
 *  @date 2018-09-19
 */
/*!
**  @addtogroup SPI_module SPI module documentation
**  @{
*/

// new types
#include "types.h"
#include "PE_Types.h"
#include "MK70F12.h"
#include "SPI.h"


/*! @brief Sets up the SPI before first use.
 *
 *  @param aSPIModule is a structure containing the operating conditions for the module.
 *  @param moduleClock The module clock in Hz.
 *  @return BOOL - true if the SPI module was successfully initialized.
 */
bool SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{
  //Enable SPI2, PortD and PortE clock gates (SCGC)
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;


  //Module Configuration Registers (MCR)
  //Set SPI to Master
  SPI2_MCR |= SPI_MCR_MSTR_MASK;

  //Disable continuous clock
  SPI2_MCR &= ~SPI_MCR_CONT_SCKE_MASK;


  //Control and Transfer Attributes Registers (CTAR)
  //Set frame size to 16 bits NOTE: CTAR should not be written while module is in running state (Fixed Value?)
  //SPI2_CTAR0 |= SPI_CTAR_FMSZ(16);

  //Set Clock polarity to inactive low
  SPI2_CTAR0 &= ~SPI_CTAR_CPOL_MASK;

  //Set data capture to leading edge
  SPI2_CTAR0 &= ~SPI_CTAR_CPHA_MASK;

  //Set MSB first
  SPI2_CTAR0 &= ~SPI_CTAR_LSBFE_MASK;

  //Set baud rate to 1Mbit/s ?Need to calculate?
  SPI2_CTAR0 |= SPI_CTAR_BR();

  //Initialise values on any chip select pins?


  return TRUE;
}

/*! @brief Selects the current slave device
 *
 * @param slaveAddress The slave device address.
 */
void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{

}

/*! @brief Simultaneously transmits and receives data.
 *
 *  @param dataTx is data to transmit.
 *  @param dataRx points to where the received data will be stored.
 */
void SPI_Exchange(const uint16_t dataTx, uint16_t* const dataRx)
{

}


/*!
** @}
*/

