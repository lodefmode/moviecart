/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  dsPIC33CK64MC105
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB 	          :  MPLAB X v5.45
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/

#include <string.h> // memset

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/memory/flash.h"
#include "mcc_generated_files/interrupt_manager.h"


#if 1
#include "mcc_generated_files/clock.h"
#define FOSC    (CLOCK_SystemFrequencyGet())
#define FCY     (CLOCK_InstructionFrequencyGet())
#include <libpic30.h>
#endif


#include "../defines.h"

__attribute__((section(".bootloader"),space(prog))) void main2();

__attribute__((section(".bootloaderSafe"),space(prog))) void main3();

__attribute__((section(".bootloaderSafe"),space(prog)))
void
flash_led(uint8_t num)
{
	uint8_t	i;

	for (i=0; i<num; i++)
    {
		TESTA0_LOW			// on
         __delay_ms(150);
        
		TESTA0_HIGH			// off
        __delay_ms(150);
    }
	
	if (!num)
		__delay_ms(500);

	__delay_ms(500);
}

__attribute__((section(".bootloaderSafe"),space(prog)))
void
resetNewCodeAndFlash(uint8_t num)
{
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    FLASH_ErasePage((uint16_t)&main2);
//	FLASH_WriteDoubleWord24((uint16_t)&main2, 0xFFFFFF, 0xFFFFFF);
    FLASH_Lock();

    while(1)
    {
        flash_led(0);
        flash_led(num);
        flash_led(1);
    }
}


/*
                         Main application
 */



// code is written to program space at this time, copy it over
// Any use of heap data will corrupt existing structures, so do not exit from this routine

// format:
// crc:       24 bit value // includes version, dest, length + data
// version:   24 bit value
// dest:      24 bit value
// length:    24 bit value // length in (24 bit) values following this section

//extern const void*	payloadData;

//void flash_led(uint8_t num);
//void resetNewCode();

// These will actually be overwritten with identical payload code
__attribute__((section(".magicIDSection"),space(prog))) const uint32_t magicID = 0x1357c0de;
__attribute__((section(".firmwareIDSection"),space(prog))) const uint32_t firmwareID = 0x000001;



__attribute__((section(".bootloaderSafe"),space(prog)))
void
main3()
{
	INTERRUPT_GlobalDisable();

//	uint32_t	src = (uint16_t)&payloadData;
	uint32_t	src = 0x5800;

	uint32_t	expected_crc = FLASH_ReadWord24(src + 0);
	uint32_t	version = FLASH_ReadWord24(src + 2);
	uint32_t	dst 	= FLASH_ReadWord24(src + 4);
	uint16_t	olen	= FLASH_ReadWord16(src + 6);

	while (version != 1)
	{
		flash_led(0);
		flash_led(3);
		flash_led(2);
	}

	// YES this may erase critical code, but unavoidable as early firmware didn't setup AIVT etc
	
	// calc checksum

    uint32_t crc = 0xFFFFFFFF;
    for (uint16_t i=1; i<(olen+4); i++) 	// skip crc
	{
		uint32_t v2 = FLASH_ReadWord24(src + 2*i);

        crc ^= v2;
        for (uint8_t j=0; j<24; j++)
		{
            if (crc & 1) 
                crc = (crc >> 1) ^ 0xEDB88320;
            else 
                crc >>= 1;
        }
    }
    crc = crc ^ 0xFFFFFFFF;
	crc &= 0xFFFFFF;	// 24 bits

	while (crc != expected_crc)
	{
		flash_led(0);
		flash_led(3);
		flash_led(4);
	}

	// now write it out

	FLASH_Unlock(FLASH_UNLOCK_KEY);

	uint32_t	erased_pc = dst;

    for (uint16_t i=0; i<olen; i+=2)
	{
		uint32_t	v1 = FLASH_ReadWord24(src + 2*(i+4));	// skip crc, version, dst, length
		uint32_t	v2 = FLASH_ReadWord24(src + 2*(i+5));

		// make some space
		while (erased_pc < (dst + 4))
		{
			if (!FLASH_ErasePage(erased_pc))
				goto bad;
			erased_pc += FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS;	// 2K
		}
				
		// write
		if (!FLASH_WriteDoubleWord24(dst, v1, v2))
			goto bad;
			
		// confirm
		if (FLASH_ReadWord24(dst+0) != v1)
			goto bad;

		if (FLASH_ReadWord24(dst+2) != v2)
			goto bad;

		dst  += 4;
			
		// overwriting configuration memory?
		if (dst >= 0xAF00)
			goto bad;
	}

	FLASH_Lock();

//good:

	resetNewCodeAndFlash(2);

bad:

	// not much we can do here, likely bricked..
	resetNewCodeAndFlash(4);
}


__attribute__((section(".bootloader"),space(prog)))
void
main2(void)
{   
    // earlier versions did goto, not call
    asm("nop");
    asm("nop");
    
	// quickly exit this section as it will be erased so regular execution doesn't find it next startup
	main3();
}

int
main(void)
{
    // initialize the device
    SYSTEM_Initialize();	// not used in final, already setup
	main2();
}

/**
 End of File
*/

