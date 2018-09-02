/*! @file
 *
 *  @brief Routines for erasing and writing to the Flash.
 *
 *  This contains the functions needed for accessing the internal Flash.
 *
 *  @author Amir and Joseph
 *  @date 2018-08-16
 */
/*!
**  @addtogroup Flash_module Flash module documentation
**  @{
*/
// header files used
#include "Flash.h"
#include "MK70F12.h"
#include "PE_types.h"

//max amount of data to store in TFCCOB
#define FCCOB_MAX_DATA 8


typedef struct
{
  uint8_t command;      /*!< The command. */

  struct
  {
    uint8_t address1,   /*!< Portion of the address. The most significant bytes of the address. */
    	    address2,   /*!< Portion of the address. */
	    address3;   /*!< Portion of the address. The least significant bytes of the address */
  } address;

  uint8_t data[FCCOB_MAX_DATA]; /*!< Array of the entire 64-bits of data. */

} TFCCOB;

/*************************function prototypes***************************/
static bool LaunchCommand(const TFCCOB* commonCommandObject);

static bool EraseSector(void);

static bool WritePhrase(const uint32_t address, const uint64_t phrase);

static bool ModifyPhrase(const uint32_t address, const uint64_t phrase);

static bool LoadData(TFCCOB* commonCommandObject, const uint64_t data);

static bool LoadAddress(uint32_t address, TFCCOB* commonCommandObject);
/***********************************************************************/

bool Flash_Init(void)
{
  return TRUE;
}


bool Flash_AllocateVar(volatile void** variable, const uint8_t size)
{
  //stores allocated address as a '1' and available address as '0'
  static uint8_t addressAllocationStorage = 0;
  // bit shifted to check each bit of addressAllocationStorage
  uint8_t allocationCheck = 1;

 // will assign an address to 'variable' depending on 'size'
  switch (size)
  {
    //checks each available address one by one until one is available. FALSE if non is available
    case 1:
      for (uint32_t start = FLASH_DATA_START; start <= FLASH_DATA_END; start++)
	{
	  if (!(addressAllocationStorage & allocationCheck))
	    {
	      *variable = (void**) start;
	      addressAllocationStorage |= allocationCheck;
	      return TRUE;
	    }
	  allocationCheck <<= 1;
	}
      break;

    //Checks each even address whether it has been assigned and if not, it will check the address right after to see if it is assigned
    //FALSE if non is available
    case 2:
      for (uint32_t start = FLASH_DATA_START; start <= FLASH_DATA_END; start += 2)
      	{
      	  if (!((addressAllocationStorage & allocationCheck) || (addressAllocationStorage & (allocationCheck << 1))))
      	    {
      	      *variable = (void**) start;
      	      addressAllocationStorage |= allocationCheck | (allocationCheck << 1);
      	      return TRUE;
      	    }
      	  allocationCheck <<= 2;
      	}
      break;

    //Checks each address divisible by 4, and the 3 addresses that comes after it if they are assigned. FALSE if non is available
    case 4:
      for (uint32_t start = FLASH_DATA_START; start <= FLASH_DATA_END; start += 4)
      	{
	  for (uint32_t i = start; i < start + 3; i++)
	    {
	      if (addressAllocationStorage & allocationCheck)
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
	      addressAllocationStorage |= allocationCheck | (allocationCheck >> 1) | (allocationCheck >> 2) | (allocationCheck >> 3);
	      return TRUE;
	    }
      	}
      break;
  }

  return FALSE;
}


bool Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{
  if ((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END && !((uint32_t)address % 4))
    {
      uint64union_t addressPosition;
      uint32_t temp = (uint32_t)address;
      // Reads the whole 64 bits into a temporary variable.
      addressPosition.l = _FP(temp & ~0x0F);

      //Writes in 32 bits to the high or low part of the temp variable depending on the address
      if(((uint32_t) address / 4) % 2)
	addressPosition.s.Hi = data;
      else
	addressPosition.s.Lo = data;

	//Calls ModifyPhrase, passing in the temp variable to write to the flash
	return ModifyPhrase((temp & ~0x0F), addressPosition.l);
    }
  return FALSE;
}


bool Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
  if ((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END && !((uint32_t)address % 2))
    {
      uint32_t temp = (uint32_t)address;
      uint32union_t addressPosition;
      // Reads the whole 32 bits into a temporary variable.
      addressPosition.l = _FW(temp & ~0x03);

      //Writes in 16 bits to the high or low part of the temp variable depending on the address
      if(((uint32_t) address / 2) % 2)
	addressPosition.s.Hi = data;
      else
	addressPosition.s.Lo = data;

      //sends the 32 bits to Flash_Write32
      return Flash_Write32((uint32_t*)(temp & ~0x03), addressPosition.l);
    }
  return FALSE;
}


bool Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
  if ((uint32_t)address >= FLASH_DATA_START && (uint32_t)address <= FLASH_DATA_END)
    {
      uint16union_t addressPosition;
      uint32_t temp = (uint32_t)address;
      // Reads the whole 16 bits into a temporary variable.
      addressPosition.l = _FH(temp & ~0x01);

      //Writes in 8 bits to the high or low part of the temp variable depending on the address
      if ((uint32_t) address % 2)
	addressPosition.s.Hi = data;
      else
	addressPosition.s.Lo = data;

      //sends the 16 bits to Flash_Write32
      return Flash_Write16((uint16_t*)(temp & ~0x01), addressPosition.l);
    }
  return FALSE;
}


bool Flash_Erase(void)
{
  return EraseSector();
}

/*! @brief Calls the relevant functions to modify the flash.
 *
 *  @param address The address of the flash sector.
 *  @param phrase The 64-bit phrase of data to write to the sector of flash.
 *  @return bool - TRUE if Flash was written successfully, FALSE if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
static bool ModifyPhrase(const uint32_t address, const uint64_t phrase)
{
  if (Flash_Erase()) //only writes in the data if the flash was erased successfully
    return WritePhrase(address, phrase);

  return FALSE;
}

/*! @brief Erases a Sector of the Flash
 *
 *  @return bool - TRUE if Flash sector was erased
 *  @note Assumes Flash has been initialized.
 */
static bool EraseSector(void)
{
  uint32_t address = FLASH_DATA_START;
  TFCCOB fccob;

  //loads the command and address in fccob struct, then calls launch command to execute the steps
  fccob.command = 0x09;
  LoadAddress(address, &fccob);
  return LaunchCommand(&fccob);
}

/*! @brief Executes a command to do something to the flash
 *
 *  @param commonCommandObject to a TFCCOB variable with all information required to load the CCOB registers
 *  @return bool - TRUE if command was successfully executed
 *  @note Assumes Flash has been initialized.
 */
static bool LaunchCommand(const TFCCOB* commonCommandObject)
{
  for (;;)
    {
      if (FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK) //check if the previous command is done executing
	break;
    }

  //Turns off the Flash Access Error Flag and Flash Protection Violation Flag by writing 1
  if (FTFE_FSTAT & FTFE_FSTAT_ACCERR_MASK)
    FTFE_FSTAT |= FTFE_FSTAT_ACCERR_MASK;
  if (FTFE_FSTAT & FTFE_FSTAT_FPVIOL_MASK)
    FTFE_FSTAT |= FTFE_FSTAT_FPVIOL_MASK;

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

  // set ccif bit to 0 to launch the command
  FTFE_FSTAT = FTFE_FSTAT_CCIF_MASK;

  for (;;)
      {
        if (FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK) //waits for the command execution to complete
          return TRUE;
      }
}

/*! @brief Writes 64-bits to a sector of the Flash
 *
 *  @param address integer value of the sector address
 *  @param phrase the 64-bits of data to be written to the flash
 *  @return bool - TRUE if flash was successfully written, FALSE if there was a programming error
 *  @note Assumes Flash has been initialized.
 */
static bool WritePhrase(const uint32_t address, const uint64_t phrase)
{
  TFCCOB fccob;
  //loading the fccob variable with the command, address and data.
  fccob.command = 0x07;
  LoadAddress(address, &fccob);
  LoadData(&fccob, phrase);

  return LaunchCommand(&fccob);
}

/*! @brief Loads the address into the TFCCOB variable
 *
 *  @param address integer value of the sector address
 *  @param commonCommandObject pointer of a TFCCOB variable to store the address
 *  @return bool - TRUE if the address was loaded in TFCCOB variable
 */
static bool LoadAddress(uint32_t address, TFCCOB* commonCommandObject)
{
  commonCommandObject->address.address3 = (uint8_t) address;
  commonCommandObject->address.address2 = (uint8_t) (address >> 8);
  commonCommandObject->address.address1 = (uint8_t) (address >> 16);
  return TRUE;
}

/*! @brief Loads the data into the TFCCOB variable.
 *
 *  @param data to be loaded in the TFCCOB variable.
 *  @param commonCommandObject pointer of a TFCCOB variable to store the data.
 *  @return bool - TRUE if the data was loaded in TFCCOB variable.
 */
static bool LoadData(TFCCOB* commonCommandObject, const uint64_t data)
{
  for (uint8_t i = 0; i < 8; i++)
    {
      commonCommandObject->data[i] = data >> (i * 8);
    }
 return TRUE;
}


/*!
** @}
*/
