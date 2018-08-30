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
#include "PE_types.h"


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

#define FCCOB_MAX_DATA 8

static uint8_t AddressAllocationStorage = 0;

typedef struct
{
  uint8_t command;

  struct
  {
    uint8_t address1,
    	    address2,
	    address3;
  } address;

  uint8_t data[FCCOB_MAX_DATA];

} TFCCOB;

static bool LaunchCommand(const TFCCOB* commonCommandObject);

static bool EraseSector(void);

static bool WritePhrase(const uint32_t address, const uint64_t phrase);

static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase);

static bool LoadData(TFCCOB* commonCommandObject, const uint64_t data);

static bool LoadAddress(uint32_t address, TFCCOB* commonCommandObject);


/*! @brief Enables the Flash module.
 *
 *  @return bool - TRUE if the Flash was setup successfully.
 */
bool Flash_Init(void)
{
  return TRUE;
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
  uint8_t allocationCheck = 1;

  switch (size)
  {
    case 1:
      for (uint32_t start = FLASH_DATA_START; start <= FLASH_DATA_END; start++)
	{
	  if (!(AddressAllocationStorage & allocationCheck))
	    {
	      *variable = (void**) start;
	      AddressAllocationStorage |= allocationCheck;
	      return TRUE;
	    }
	  allocationCheck <<= 1;
	}
      break;

    case 2:
      for (uint32_t start = FLASH_DATA_START; start <= FLASH_DATA_END; start += 2)
      	{
      	  if (!((AddressAllocationStorage & allocationCheck) || (AddressAllocationStorage & (allocationCheck << 1))))
      	    {
      	      *variable = (void**) start;
      	      AddressAllocationStorage |= allocationCheck | (allocationCheck << 1);
      	      return TRUE;
      	    }
      	  allocationCheck <<= 2;
      	}
      break;

    case 4:
      for (uint32_t start = FLASH_DATA_START; start <= FLASH_DATA_END; start += 4)
      	{
	  for (uint32_t i = start; i < start + 3; i++)
	    {
	      if (AddressAllocationStorage & allocationCheck)
		{
		  allocationCheck = 16;
		  break;
		}
	      else
		allocationCheck <<= 1;
	    }
	  if (allocationCheck == 8 || allocationCheck == 128)
	    {
	      *variable = (void**) start;
	      AddressAllocationStorage |= allocationCheck | (allocationCheck >> 1) | (allocationCheck >> 2) | (allocationCheck >> 3);
	      return TRUE;
	    }
      	}
      break;
  }

  return FALSE;
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
  if ((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END)
    {
      // First read the whole 64 bits into a temporary variable.
      uint64union_t addressPosition;
      uint32_t temp = (uint32_t)address;
      addressPosition.l = *((uint64_t*)(temp & ~0x0F));

      // Then write in your 32 bits to the high or low part of the temp variable
      if(((uint32_t) address / 4) % 2)
	addressPosition.s.Hi = data;
      else
	addressPosition.s.Lo = data;

	// Then call flash write 64, passing in the temp variable
	return ModifyPhrase((temp & ~0x0F), addressPosition);
    }
  return FALSE;
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
  if ((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END)
    {
      uint32_t temp = (uint32_t)address;
      uint32union_t addressPosition;

      addressPosition.l = *((uint16_t*)(temp & ~0x03));

      if(((uint32_t) address / 2) % 2)
	addressPosition.s.Hi = data;
      else
	addressPosition.s.Lo = data;

      return Flash_Write32((uint32_t*)(temp & ~0x03), addressPosition.l);
    }
  return FALSE;
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
  if ((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END)
    {
      uint16union_t addressPosition;
      uint32_t temp = (uint32_t)address;
      addressPosition.l = *(uint8_t*)(temp & ~0x01);

      if ((uint32_t) address % 2)
	addressPosition.s.Hi = data;
      else
	addressPosition.s.Lo = data;

      return Flash_Write16((uint16_t*)(temp & ~0x01), addressPosition.l);
    }
  return FALSE;
}

/*! @brief Erases the entire Flash sector.
 *
 *  @return bool - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
bool Flash_Erase(void)
{
  return EraseSector();
}

static bool ModifyPhrase(const uint32_t address, const uint64union_t phrase)
{
  if (Flash_Erase())
    return WritePhrase(address, phrase.l);

  return FALSE;
}

static bool EraseSector(void)
{
  uint32_t address = FLASH_DATA_START;
  TFCCOB fccob;

  fccob.command = 0x09;
  LoadAddress(address, &fccob);
  return LaunchCommand(&fccob);
}

static bool LaunchCommand(const TFCCOB* commonCommandObject)
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
  FTFE_FCCOB7 = commonCommandObject->data[0];
  FTFE_FCCOB6 = commonCommandObject->data[1];
  FTFE_FCCOB5 = commonCommandObject->data[2];
  FTFE_FCCOB4 = commonCommandObject->data[3];
  FTFE_FCCOBB = commonCommandObject->data[4];
  FTFE_FCCOBA = commonCommandObject->data[5];
  FTFE_FCCOB9 = commonCommandObject->data[6];
  FTFE_FCCOB8 = commonCommandObject->data[7];

  // set ccif bit to 0
  FTFE_FSTAT = FTFE_FSTAT_CCIF_MASK;
  return TRUE;
}

static bool WritePhrase(const uint32_t address, const uint64_t phrase)
{
  TFCCOB fccob;

  fccob.command = 0x07;
  LoadAddress(address, &fccob);
  LoadData(&fccob, phrase);

  return LaunchCommand(&fccob);
}


static bool LoadAddress(uint32_t address, TFCCOB* commonCommandObject)
{
  commonCommandObject->address.address3 = (uint8_t) address;
  commonCommandObject->address.address2 = (uint8_t) (address >> 8);
  commonCommandObject->address.address1 = (uint8_t) (address >> 16);
}

static bool LoadData(TFCCOB* commonCommandObject, const uint64_t data)
{
  for (uint8_t i = 0; i < 8; i++)
    {
      commonCommandObject->data[i] = data >> (i * 8);
    }
 return TRUE;
}











