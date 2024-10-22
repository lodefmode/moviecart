/**
  FLASH Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    flash.h

  @Summary
    This is the generated header file for the FLASH driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for FLASH.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  dsPIC33CK64MC105
        Driver Version    :  1.00
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB 	          :  MPLAB X v5.45
*/

#ifndef FLASH_H
#define FLASH_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#define FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS  128U
#define FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS 1024U

#define FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS  (FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS*2U)
#define FLASH_WRITE_ROW_SIZE_IN_PC_UNITS (FLASH_WRITE_ROW_SIZE_IN_INSTRUCTIONS*2U)
#define FLASH_HAS_ECC  1

#define FLASH_UNLOCK_KEY 0x00AA0055

#define FLASH_ERASE_PAGE_MASK (~((FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS*2U) - 1U)) 
void     FLASH_Unlock(uint32_t  key);
void     FLASH_Lock(void);

bool     FLASH_ErasePage(uint32_t address);    

uint16_t FLASH_ReadWord16(uint32_t address);
uint32_t FLASH_ReadWord24(uint32_t address);

bool     FLASH_WriteDoubleWord16(uint32_t flashAddress, uint16_t Data0, uint16_t Data1);
bool     FLASH_WriteDoubleWord24(uint32_t address, uint32_t Data0, uint32_t Data1  );

/* Program the flash one row at a time. */

/* FLASH_WriteRow24: Writes a single row of data from the location given in *data to
 *                   the flash location in address.  Since the flash is only 24 bits wide
 *                   all data in the upper 8 bits of the source will be lost .  
 *                   The address in *data must be row aligned.
 *                   returns true if successful */

bool     FLASH_WriteRow24(uint32_t flashAddress, uint32_t *data);

/* FLASH_WriteRow16: Writes a single row of data from the location in given in *data to
 *                   to the flash location in address. Each 16 bit source data 
 *                   word is stored in the lower 16 bits of each flash entry and the 
 *                   upper 8 bits of the flash is not programmed. 
 *                   The address in *data must be row aligned.
 *                   returns true if successful */
bool     FLASH_WriteRow16(uint32_t address, uint16_t *data);


uint16_t FLASH_GetErasePageOffset(uint32_t address);
uint32_t FLASH_GetErasePageAddress(uint32_t address);




#endif	/* FLASH_H */

