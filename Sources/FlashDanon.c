#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// header file

// FLASH data access
#define _FB(flashAddress)  *(uint8_t  volatile *)(flashAddress)
#define _FH(flashAddress)  *(uint16_t volatile *)(flashAddress)
#define _FW(flashAddress)  *(uint32_t volatile *)(flashAddress)
#define _FP(flashAddress)  *(uint64_t volatile *)(flashAddress)

// Address of the start of the Flash block we are using for data storage
#define FLASH_DATA_START 0x00080000LU
// Address of the end of the Flash block we are using for data storage
#define FLASH_DATA_END   0x00080007LU
// header file

// student code
#define TRUE true
#define FALSE false
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


// student code
int main()
{
    void *testPointer;
    bool successVar;
    
    successVar = Flash_AllocateVar((volatile void **)&testPointer, 0x01);
    printf("allocate 0x01, successVar = %u, testPointer = %x\n", successVar, (uint32_t)testPointer);
    
    successVar = Flash_AllocateVar((volatile void **)&testPointer, 0x04);
    printf("allocate 0x04, successVar = %u, testPointer = %x\n", successVar, (uint32_t)testPointer);
    
    successVar = Flash_AllocateVar((volatile void **)&testPointer, 0x02);
    printf("allocate 0x02, successVar = %u, testPointer = %x\n", successVar, (uint32_t)testPointer);
    
    successVar = Flash_AllocateVar((volatile void **)&testPointer, 0x01);
    printf("allocate 0x01, successVar = %u, testPointer = %x\n", successVar, (uint32_t)testPointer);
    
    successVar = Flash_AllocateVar((volatile void **)&testPointer, 0x01);
    printf("allocate 0x01, successVar = %u, testPointer = %x\n", successVar, (uint32_t)testPointer);

    return 0;
}

// Expected Output
// $gcc -m32 -o main *.c
// $main
// allocate 0x01, successVar = 1, testPointer = 80000
// allocate 0x04, successVar = 1, testPointer = 80004
// allocate 0x02, successVar = 1, testPointer = 80002
// allocate 0x01, successVar = 1, testPointer = 80001
// allocate 0x01, successVar = 0, testPointer = 0

// Student Output
// $gcc -m32 -o main *.c
// $main
// allocate 0x01, successVar = 1, testPointer = 80000
// allocate 0x04, successVar = 1, testPointer = 80004
// allocate 0x02, successVar = 1, testPointer = 80002
// allocate 0x01, successVar = 1, testPointer = 80001
// allocate 0x01, successVar = 0, testPointer = untouched