/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.65
        Device            :  PIC18F47K42
        Driver Version    :  2.00
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
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

#include "mcc_generated_files/mcc.h"


#if 0

pic18f47k42

NOTES:
-Can only reach 4.25 volts without external source.

-RE3 is input only

-RB7 (SRAM1_W) and RB6 TLATCH are forced to inputs during h/w debugging

-SRAM doesn't allow write when being read (1023) ????
 -state of address lines won't be known
 -use CE of the output side
        
#endif

#include "defines.h"
    
extern void runStateMachine(void);
extern bool initState(void);
extern void stopTitleScreen(void);

void
quick_flash(uint8_t num)
{
    uint8_t i;
    
    TEST_LED_OFF
    TEST_LED_OFF
    TEST_LED_OFF
    TEST_LED_OFF

    
    for (i=0; i<num; i++)
    {
        TEST_LED_ON
        TEST_LED_ON

        TEST_LED_OFF
        TEST_LED_OFF        
    }

    TEST_LED_OFF
    TEST_LED_OFF
    TEST_LED_OFF
    TEST_LED_OFF


}


void
test_flash(uint8_t num)
{
    uint8_t i;
    
    TEST_LED_OFF
    
    for (i=0; i<num; i++)
    {
        TEST_LED_ON
        __delay_ms(200);
        TEST_LED_OFF
        __delay_ms(300);
    }
    __delay_ms(500);
}

unsigned char
readSramSlow(uint16_t address)
{
    unsigned char   dd;
    
    ADDR_LO = LO7_BYTE(address);
    ADDR_HI = HI3_BYTE(address);
    
    DATA_TRIS = 0xff;  // input
    SRAM_OE = 0;      // enable output
    dd = DATA_BUS_IN;
    SRAM_OE = 1;      // disable output
    
    DATA_TRIS = 0;      // output
    
    return dd;
} 

#include "../output/core_data.h"

// loop
#if 0
	; setup  -- FB   --- $00    // nmi (brk)
	; setup  FC FD   $FB $F3	// reset
	; setup  FE FF   $FB $F3	// irq/brk
	; (should start looping with those 5 consecutive bytes)
	;
	; setup  remaining bytes
	; when ready...
	; pause long enough to make sure 6502 is looping  
	; setup  FC FD with original address (reset)
	
	; while  FB      $20        (only 1 bit difference, jsr reset-vector)
	; optional: main_start begins with 1024cycle delay

	org $F3FA   ; 1k
reset_loop
;	.byte $FF, $00			;NMI   		// not-used   ,   BRK(00) / JSR(20)
;	.word reset_loop+1 		;RESET		// jump to FFFB=00 -> brk
;	.word reset_loop+1 		;IRQ/BRK	// after first brk 

#endif

void
endLoop()
{
	{
		BEGIN_WRITE()
		ADDR_HI = 7;

		// 6507 should be repeating:  brk -> brk vector 

		// reset vector should be unused, fill with original address
		// original break vector gets lost though

		ADDR_LO = (0xfc & 127);
		DATA_BUS_OUT = core_data[0xfffc & 1023];
		SRAM_WE = 0;
		SRAM_WE = 1;

		ADDR_LO = (0xfd & 127);
		DATA_BUS_OUT = core_data[0xfffd & 1023];
		SRAM_WE = 0;
		SRAM_WE = 1;

		END_WRITE()
	}

	// now change the BRK(00) to JSR(20) to JSR into original reset vector

	while(1)
	{
        BEGIN_WRITE()
		ADDR_HI = 7;

		ADDR_LO = (0xfb & 127);
		DATA_BUS_OUT = 0x20;	// jsr, 1 bit off 0x00
		SRAM_WE = 0;
		SRAM_WE = 1;

		END_WRITE()

		// but may be in conflict if being read, so confirm first
        if (readSramSlow(0xfffb & 1023) == 0x20)
			break;
	}

	// some roms use the data in the vector section during gameplay, so restore it
	// after we know its been read above
    // may still a race condition if its read immediately after jsr above for conditional data
	{
		BEGIN_WRITE()
		ADDR_HI = 7;

        ADDR_LO = (0xfa & 127);
		DATA_BUS_OUT = core_data[0xfffa & 1023];  // nmi lo
		SRAM_WE = 0;
		SRAM_WE = 1;

        ADDR_LO = (0xfb & 127);
		DATA_BUS_OUT = core_data[0xfffb & 1023];  // nmi hi
		SRAM_WE = 0;
		SRAM_WE = 1;

        
		ADDR_LO = (0xfe & 127);
		DATA_BUS_OUT = core_data[0xfffe & 1023];  // brk lo
		SRAM_WE = 0;
		SRAM_WE = 1;

		ADDR_LO = (0xff & 127);
		DATA_BUS_OUT = core_data[0xffff & 1023];  // brk hi
		SRAM_WE = 0;
		SRAM_WE = 1;

		END_WRITE()
	}
}

void
startLoop()
{
	BEGIN_WRITE()
	ADDR_HI = 7;

		// 0xFA , not needed

		ADDR_LO = (0xFB & 127);		// nmi hi			// 0xF3FB = 0x00 (brk)
		DATA_BUS_OUT = 0x00;
		SRAM_WE = 0;
		SRAM_WE = 1;


		ADDR_LO = (0xFC & 127);		// reset lo			// reset = 0xF3FB
		DATA_BUS_OUT = 0xFB;
		SRAM_WE = 0;
		SRAM_WE = 1;
		ADDR_LO = (0xFD & 127);		// reset hi
		DATA_BUS_OUT = 0xF3;
		SRAM_WE = 0;
		SRAM_WE = 1;


		ADDR_LO = (0xFE & 127);		// brk lo			// brk = 0xF3FB
		DATA_BUS_OUT = 0xFB;
		SRAM_WE = 0;
		SRAM_WE = 1;
		ADDR_LO = (0xFF & 127);		// brk hi
		DATA_BUS_OUT = 0xF3;
		SRAM_WE = 0;
		SRAM_WE = 1;

	END_WRITE()
}

bool
verifyStartLoop()
{
    if (readSramSlow(0xFFFB) != 0x00)
        return false;

    if (readSramSlow(0xFFFC) != 0xFB)
        return false;
    
    if (readSramSlow(0xFFFD) != 0xF3)
        return false;

    if (readSramSlow(0xFFFE) != 0xFB)
        return false;
    
    if (readSramSlow(0xFFFF) != 0xF3)
        return false;

    return true;
}

void
fillPreCore()
{
	// dump of all 8 pages, minus vector
	
	uint16_t	index = 0;

	BEGIN_WRITE()

	// don't fill vector table
    for (index=0; index<1024-VECTOR_SIZE; index++)
    {
        SET_WRITE_PAGE(index);
        WRITE_DATA(index, core_data[index]);
    }
    
	END_WRITE()
}

bool
verifyPreCore()
{
    unsigned char   dd;
    int16_t         addr;

    // verify
    for (addr=(1023 - VECTOR_SIZE); addr>=0; addr--)
    {
        dd = core_data[addr];
        if (readSramSlow(addr) != dd)
			return false;
    }

	return true;
}
    
/////////////////////////////////////////////////////////////////////////////////////////////


/*
                         Main application
 */


//uint8_t test_read[512];

void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();
    
    
    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts
    // Use the following macros to:

    // Disable the Global Interrupts
  //  INTERRUPT_GlobalInterruptDisable();
    
    // Add your application code
    
    SRAM_WE = 1;      // disable write      

	///////////////////////////////////////////////////////

    // do this as quickly as possible!
    SRAM2_CE = 1;   // avoid collisions 
    while(1)
    {
        startLoop();
        if (verifyStartLoop())
            break;
    }
    SRAM2_CE = 0;   // enable output again
    
    quick_flash(6);

	// now fill in rest
    while(1)
    {
        fillPreCore();  
        if (verifyPreCore())
            break;
    }

    quick_flash(5);
    

	// now turn off the loop..
	// should be in title screen
    endLoop();

    
    quick_flash(4);

	// setup stream
    TEST_LED_ON;
	if (!initState())
        test_flash(3);
    TEST_LED_OFF;
    
    quick_flash(3);
    
    // delay to make sure processor is running, title screen
    test_flash(2);

	// turn off title screen
    stopTitleScreen();

    
	// make sure its not in title screen (at least 1/60th)
    __delay_ms(50);

    quick_flash(2);
    
    runStateMachine();
}

/**
 End of File
*/
