/*! @file
 *
 *  @brief Routines for erasing and writing to the Flash.
 *
 *  This contains the functions needed for accessing the internal Flash.
 *
 *  @author Amir and Joseph
 *  @date 2018-08-16
 */

// new types
#include "flash.h"
#include "MK70F12.h"


/*
 * 0x0008_0000
 * 0x0008_0001
 * 0x0008_0002
 * 0x0008_0003
 * 0x0008_0004
 * 0x0008_0005
 * 0x0008_0006
 * 0x0008_0007
 */


typedef struct
{
  uint8_t command;

  struct
  {
    uint8_t address1,
    	    address2,
	    address3;
  } address;

  struct
  {
    uint8_t byte0,
	    byte1,
	    byte2,
	    byte3,
	    byte4,
	    byte5,
	    byte6,
	    byte7;
  } data;

} TFCCOB;

static bool LaunchCommand(TFCCOB* commonCommandObject);

static bool EraseSector(const uint32_t address);

/*! @brief Enables the Flash module.
 *
 *  @return bool - TRUE if the Flash was setup successfully.
 */
bool Flash_Init(void)
{
  return 0;
}

/*! @brief Allocates space for a non-volatile variable in the Flash memory.
 *
 *  @param variable is the address of a pointer to a variable that is to be allocated space in Flash memory.
 *         The pointer will be allocated to a relevant address:
 *         If the variable is a byte, then any address.
 *         If the variable is a half-word, then an even address.
 *         If the variable is a word, then an address divisible by 4.
 *         This allows the resulting variable to be used with the relevant Flash_Write function which assumes a certain memory address.
 *         e.g. a 16-bit variable will be on an even address
 *  @param size The size, in bytes, of the variable that is to be allocated space in the Flash memory. Valid values are 1, 2 and 4.
 *  @return bool - TRUE if the variable was allocated space in the Flash memory.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_AllocateVar(volatile void** variable, const uint8_t size)
{
  static uint8_t adressAllocationStorage;

  switch (size)
  {
    case 1:
      break;

    case 2: break;

    case 4: break;
  }
}

/*! @brief Writes a 32-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 32-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if address is not aligned to a 4-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{

}

/*! @brief Writes a 16-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 16-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if address is not aligned to a 2-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
  //Flash_Write32();
}

/*! @brief Writes an 8-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 8-bit data to write.
 *  @return bool - TRUE if Flash was written successfully, FALSE if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{

}

/*! @brief Erases the entire Flash sector.
 *
 *  @return bool - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Erase(void)
{
  EraseSector((uint32_t) FLASH_DATA_START);
}

static bool LaunchCommand(TFCCOB* commonCommandObject)
{
  for (;;)
    {
      if (FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK)
	break;
    }

  if (FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK)
    FTFE_FSTAT &= ~FTFE_FSTAT_ACCERR_MASK;

  if (FTFE_FSTAT & FTFE_FSTAT_FPVIOL_MASK)
    FTFE_FSTAT &= ~FTFE_FSTAT_FPVIOL_MASK;

  //load command in register
  FTFE_FCCOB0 = commonCommandObject->command;
  //load address in register
  FTFE_FCCOB1 = commonCommandObject->address.address1;
  FTFE_FCCOB2 = commonCommandObject->address.address2;
  FTFE_FCCOB3 = commonCommandObject->address.address3;
  //load data in register
  FTFE_FCCOB4 = commonCommandObject->data.byte0;
  FTFE_FCCOB5 = commonCommandObject->data.byte1;
  FTFE_FCCOB6 = commonCommandObject->data.byte2;
  FTFE_FCCOB7 = commonCommandObject->data.byte3;
  FTFE_FCCOB8 = commonCommandObject->data.byte4;
  FTFE_FCCOB9 = commonCommandObject->data.byte5;
  FTFE_FCCOBA = commonCommandObject->data.byte6;
  FTFE_FCCOBB = commonCommandObject->data.byte7;

  FTFE_FSTAT &= ~FTFE_FSTAT_CCIF_MASK;
}

static bool EraseSector(const uint32_t address)
{
  TFCCOB fccob;

  fccob.command = 0x09;
  fccob.address.address3 = (uint8_t) address;
  fccob.address.address2 = (uint8_t) (address >> 8);
  fccob.address.address1 = (uint8_t) (address >> 16);
  return LaunchCommand(&fccob);
}
