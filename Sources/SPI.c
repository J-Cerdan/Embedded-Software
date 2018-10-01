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
#include <stdlib.h>
#include "types.h"
#include "PE_Types.h"
#include "MK70F12.h"
#include "SPI.h"

//Variable to store data and commands
uint32union_t PUSHR_DATA;

/*! @brief Exhaustive search function to find PDT and DT values.
 *
 *  @param moduleClock The module clock in Hz.
 *  @return none.
 */
void CalculateDelay(uint32_t moduleClock)
{
  //Initialisation of variables
  uint8_t microsecondsClock;

  uint8_t pdtResult = 0;
  uint8_t dtResult = 0;
  uint16_t outcome = 0;
  uint16_t lowestOutcome = 20000;

  //All possible pdt and dt values in respective arrays
  uint8_t pdt[] = {1,3,5,7};
  uint32_t dt[] = {2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536};

  //Module clock Hz to microseconds
  microsecondsClock = 100000000/moduleClock;

  for (uint8_t i = 0; i<sizeof(pdt); i++)
    {
      for (uint8_t j = 0; j<(sizeof(dt) / 4); j++)
	{
	  //Comparison of calculated to desired value
	  outcome = (microsecondsClock*pdt[i]*dt[j]) - 500;

	  if (outcome < lowestOutcome)
	    {
	      pdtResult = i;
	      dtResult = j;
	      lowestOutcome = outcome;
	    }
	}
    }

  //Store pdt and dt resulting values into CTAR0.
  SPI2_CTAR0 |= SPI_CTAR_PDT(pdtResult);
  SPI2_CTAR0 |= SPI_CTAR_DT(dtResult);
}

/*! @brief Exhaustive search function to find PBR and BR values.
 *
 *  @param aSPIModule is a structure containing the operating conditions for the module.
 *  @param moduleClock The module clock in Hz.
 *  @return none.
 */
void CalculateBaud(TSPIModule const aSPIModule, uint32_t moduleClock)
{
  //Initialisation of variables
  uint8_t pbrResult = 0;
  uint8_t brResult = 0;
  uint8_t dbr = 0;
  uint32_t outcome = 0;
  uint32_t lowestOutcome = 10000000;

  //All possible pbr and br values in respective arrays
  uint8_t pbr[] = {2,3,5,7};
  uint16_t br[] = {2,4,6,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

  for (uint8_t i = 0; i < sizeof(pbr); i++)
    {
      for (uint8_t j = 0; j < (sizeof(br) / 2); j++)
	{
	  //Comparison of calculated to desired value
	  outcome = ((moduleClock * (1+dbr)) / (pbr[i] * br[j])) - aSPIModule.baudRate;

	  if (outcome < lowestOutcome)
	    {
	      pbrResult = i;
	      brResult = j;
	      lowestOutcome = outcome;
	    }
	}
    }

  //Store pdt and dt resulting values into CTAR0.
  SPI2_CTAR0 &= ~SPI_CTAR_DBR_MASK;
  SPI2_CTAR0 |= SPI_CTAR_PBR(pbrResult);
  SPI2_CTAR0 |= SPI_CTAR_BR(brResult);
}


bool SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{

  //Utilise if statements to enable values

  //Enable SPI2, PortD and PortE clock gates (SCGC)
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK;
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; //slave in, slave out selection
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; //chip select 17/18

  //Initialise values on any chip select pins
  PORTD_PCR11 |= PORT_PCR_MUX(2);
  PORTD_PCR12 |= PORT_PCR_MUX(2);
  PORTD_PCR13 |= PORT_PCR_MUX(2);
  PORTD_PCR14 |= PORT_PCR_MUX(2);
  PORTD_PCR15 |= PORT_PCR_MUX(2);

  //Enable ports for GPIO 7 and 8
  PORTE_PCR5 |= PORT_PCR_MUX(1);
  PORTE_PCR27 |= PORT_PCR_MUX(1);

  //GPIO ports to clear and enable output
  GPIOE_PCOR = (1 << 27) | (1 << 5);
  GPIOE_PDDR |= (1 << 27) | (1 << 5);

  //Module Configuration Registers (MCR)
  //Control and Transfer Attributes Registers (CTAR)
  //NOTE: CTAR should not be written while module is in running state

  //Select CTAR0
  SPI2_PUSHR |= SPI_PUSHR_CTAS(0);

  //MCR Configurations
  SPI2_MCR |= SPI_MCR_FRZ_MASK;
  SPI2_MCR &= ~SPI_MCR_MDIS_MASK;
  SPI2_MCR |= SPI_MCR_PCSIS(1);
  SPI2_MCR |= SPI_MCR_DIS_TXF_MASK;
  SPI2_MCR |= SPI_MCR_DIS_RXF_MASK;

  //SPI Module configurations
  if (aSPIModule != NULL)
    {
      if (aSPIModule->isMaster)
	{
	  //Set SPI to Master
	  SPI2_MCR |= SPI_MCR_MSTR_MASK;
	}
      else
	{
	  SPI2_MCR &= ~SPI_MCR_MSTR_MASK;
	}

      if (aSPIModule->continuousClock)
	{
	  SPI2_MCR |= SPI_MCR_CONT_SCKE_MASK;
	}
      else
	{
	  //Disable continuous clock
	  SPI2_MCR &= ~SPI_MCR_CONT_SCKE_MASK;
	}

      if (aSPIModule->inactiveHighClock)
      	{
	  SPI2_CTAR0 |= SPI_CTAR_CPOL_MASK;
      	}
      else
      	{
	  //Set Clock polarity to inactive low
	  SPI2_CTAR0 &= ~SPI_CTAR_CPOL_MASK;
      	}

      if (aSPIModule->changedOnLeadingClockEdge)
      	{
	  SPI2_CTAR0 |= SPI_CTAR_CPHA_MASK;
      	}
      else
      	{
	  //Set data capture to leading edge
	  SPI2_CTAR0 &= ~SPI_CTAR_CPHA_MASK;
      	}

      if (aSPIModule->LSBFirst)
      	{
	  SPI2_CTAR0 |= SPI_CTAR_LSBFE_MASK;
      	}
      else
      	{
	  //Set MSB first
	  SPI2_CTAR0 &= ~SPI_CTAR_LSBFE_MASK;

      	}

      SPI2_CTAR0 |= SPI_CTAR_FMSZ(15);

    }

  //Calculate delay and baud rate to input into registers
  CalculateDelay(moduleClock);
  CalculateBaud(*aSPIModule, moduleClock);

  //Set command values for PUSHR register for later use
  PUSHR_DATA.l = 0x00010000;

  //Enable Module
  SPI2_MCR &= ~SPI_MCR_HALT_MASK;

  return TRUE;

}


void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{
  //GPIO 7 & 8 Masks
  uint32_t GPIO7 = 1 << 27;
  uint32_t GPIO8 = 1 << 5;


  switch(slaveAddress)
  {
    //LTC2704
    case 0:
      GPIOE_PCOR |= (GPIO7 | GPIO8);
      break;

    //LTC2600
    case 1:
      GPIOE_PSOR |= GPIO7;
      GPIOE_PCOR |= GPIO8;
      break;

    //LTC2498
    case 2:
      GPIOE_PCOR |= GPIO7;
      GPIOE_PSOR |= GPIO8;
      break;

    //LTC1859
    case 3:
      GPIOE_PSOR |= (GPIO7 | GPIO8);
      break;
  }
}


void SPI_Exchange(const uint16_t dataTx, uint16_t* const dataRx)
{
  uint16_t SPIData;

  //Stores data to send
  PUSHR_DATA.s.Lo = dataTx;

  //Wait until bus is idle
  while (!(SPI2_SR & SPI_SR_TFFF_MASK))
    {/*wait*/}

  //w1c
  SPI2_SR |= SPI_SR_TFFF_MASK;

  //Push data and command to PUSHR
  SPI2_PUSHR = PUSHR_DATA.l;


  //Wait until fifo is not empty
  while (!(SPI2_SR & SPI_SR_RFDF_MASK))
    {/*wait*/}

  SPIData = (uint16_t) SPI2_POPR;
  *dataRx = SPIData;

  //w1c
  SPI2_SR |= SPI_SR_RFDF_MASK;

}

/*!
** @}
*/

