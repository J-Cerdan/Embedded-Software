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

static uint8_t SlaveAddress;


/*! @brief Sets up the SPI before first use.
 *
 *  @param aSPIModule is a structure containing the operating conditions for the module.
 *  @param moduleClock The module clock in Hz.
 *  @return BOOL - true if the SPI module was successfully initialized.
 */
bool SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{

  //Utilise if statements to enable values

  //Enable SPI2, PortD and PortE clock gates (SCGC)
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; //slave in, slave out selection
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; //chip select 17/18


  //Module Configuration Registers (MCR)
  //Control and Transfer Attributes Registers (CTAR)
  //NOTE: CTAR should not be written while module is in running state (Fixed Value?)

  if (aSPIModule != Null)
    {
      if (aSPIModule->isMaster == TRUE)
	{
	  //Set SPI to Master
	  SPI2_MCR |= SPI_MCR_MSTR_MASK;
	}
      else
	{
	  SPI2_MCR &= ~SPI_MCR_MSTR_MASK;
	}

      if (aSPIModule->continuousClock == FALSE)
	{
	  //Disable continuous clock
	  SPI2_MCR &= ~SPI_MCR_CONT_SCKE_MASK;
	}
      else
	{
	  SPI2_MCR |= SPI_MCR_CONT_SCKE_MASK;
	}

      if (aSPIModule->inactiveHighClock == FALSE)
      	{
	  //Set Clock polarity to inactive low
	  SPI2_CTAR0 &= ~SPI_CTAR_CPOL_MASK;
      	}
      else
      	{
	  SPI2_CTAR0 |= SPI_CTAR_CPOL_MASK;
      	}

      if (aSPIModule->changedOnLeadingClockEdge == FALSE)
      	{
	  //Set data capture to leading edge
	  SPI2_CTAR0 &= ~SPI_CTAR_CPHA_MASK;
      	}
      else
      	{
	  SPI2_CTAR0 |= SPI_CTAR_CPHA_MASK;
      	}

      if (aSPIModule->LSBFirst == FALSE)
      	{
	  //Set MSB first
	  SPI2_CTAR0 &= ~SPI_CTAR_LSBFE_MASK;
      	}
      else
      	{
	  SPI2_CTAR0 |= SPI_CTAR_LSBFE_MASK;
      	}

      SPI2_CTAR0 |= SPI_CTAR_FMSZ(15);

    }

  //Exhaustive search? for both delay and baud
  //baud rate = (module clock(50000000) x PBR) x [(1+DBR)/BR];
  //aSPIModule->baudRate

  //Set Delay After Transfer Scalers
  SPI2_CTAR0 |= SPI_CTAR_PDT(0);
  SPI2_CTAR0 |= SPI_CTAR_DT(256);

  //Set baud rate to 1Mbit/s ?Need to calculate?
  SPI2_CTAR0 |= SPI_CTAR_DBR();
  SPI2_CTAR0 |= SPI_CTAR_PBR();
  SPI2_CTAR0 |= SPI_CTAR_BR();

  //Initialise values on any chip select pins
  PORTD_GPCLR |= PORT_GPCLR_GPWE(11);
  PORTD_GPCLR |= PORT_GPCLR_GPWE(12);
  PORTD_GPCLR |= PORT_GPCLR_GPWE(13);
  PORTD_GPCLR |= PORT_GPCLR_GPWE(14);
  PORTD_GPCLR |= PORT_GPCLR_GPWE(15);
  PORTE_GPCHR |= PORT_GPCHR_GPWE(18);

  //Initial settings for transmitting data
  SPI2_PUSHR &= ~SPI_PUSHR_CONT_MASK;
  SPI2_PUSHR &= ~SPI_PUSHR_CTAS_MASK;
  SPI2_PUSHR &= ~SPI_PUSHR_EOQ_MASK;
  SPI2_PUSHR &= ~SPI_PUSHR_CTCNT_MASK;
  SPI2_PUSHR |= SPI_PUSHR_PCS_MASK;


  return TRUE;
}

/*! @brief Selects the current slave device
 *
 * @param slaveAddress The slave device address.
 */
void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{
  SlaveAddress = slaveAddress;
}

/*! @brief Simultaneously transmits and receives data.
 *
 *  @param dataTx is data to transmit.
 *  @param dataRx points to where the received data will be stored.
 */
void SPI_Exchange(const uint16_t dataTx, uint16_t* const dataRx)
{


}

void CalculateDelay(uint32_t moduleClock)
{
  uint8_t pdt[] = {1,3,5,7};
  uint16_t dt[] = {2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536};

  for (uint8_t i = 0; i<sizeof(pdt); i++)
    {
      for (uint8_t j = 0; j<sizeof(dt); j++)
	{

	}
    }
}


/*!
** @}
*/

