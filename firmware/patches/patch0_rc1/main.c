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
#include "mcc_generated_files/spi1.h"
#include "mcc_generated_files/memory/flash.h"
#include "mcc_generated_files/interrupt_manager.h"


#if 1
#include "mcc_generated_files/clock.h"
#define FOSC    (CLOCK_SystemFrequencyGet())
#define FCY     (CLOCK_InstructionFrequencyGet())
#include <libpic30.h>
#endif


__attribute__((section(".newcode"),space(prog))) void main2(void);

void flash_led(uint8_t num);

#include "pff.h"
#include "core.h"
#include "update.h"

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

// can be used for debugging
// 155000 Bits/sec
// 8 bits per transfer (standard)
// 2 stop bits
// no parity bit (standard)
// least significant bit sent first (standard)
// non inverted (standard)

void
serialLED(uint8_t v)
{
    uint16_t    i, j;

    // start bit
    for (i=0; i<24; i++)
		TESTA0_LOW
        
    for (j=0; j<8; j++)
    {
        if (v & (1<<j))
        {
            for (i=0; i<24; i++)
            {
				TESTA0_HIGH
            }
        }
        else
        {
            for (i=0; i<24; i++)
            {
				TESTA0_LOW
            }
        }        
    }

    // stop bit
    for (i=0; i<24*2; i++)
		TESTA0_HIGH
}

/*
                         Main application
 */


extern struct coreInfo			r_coreInfo;
extern struct fileSystemInfo	fsInfo;


extern void updateInit();


__attribute__((section(".stateVars"))) struct stateVars		state;
__attribute__((section(".mr_buffer1"))) uint8_t mr_buffer1[FIELD_SIZE];
__attribute__((section(".mr_buffer2")))uint8_t mr_buffer2[FIELD_SIZE];

// Allocate and reserve a page of flash for this test to use.  The compiler/linker will reserve this for data and not place any code here.
//static __prog__  uint8_t flashTestPage[FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS] __attribute__((space(prog),aligned(FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS)));

void
copyFromFlash(uint8_t *dst, uint32_t src, int size)
{
	while (size > 0)
	{
		uint32_t read_data = FLASH_ReadWord24(src);

		*dst++ = (read_data & 0x0000ff) >> 0;
		*dst++ = (read_data & 0x00ff00) >> 8;
		*dst++ = (read_data & 0xff0000) >> 16;

		src += 2;
		size -= 3;
	}
}

void
setupTitle()
{
	// setup frame headers quickly
	r_coreInfo.frameInfo.buffer = mr_buffer1;
	r_coreInfo.mr_frameInfo1.buffer = mr_buffer1;
	r_coreInfo.mr_frameInfo2.buffer = mr_buffer2;

	frameInitTitle(&r_coreInfo.frameInfo, 0);
	frameInitTitle(&r_coreInfo.mr_frameInfo1, 0);
	frameInitTitle(&r_coreInfo.mr_frameInfo2, 1);

	// zero everything since height may change
	memset(mr_buffer1, 0, sizeof(mr_buffer1));
	memset(mr_buffer2, 0, sizeof(mr_buffer2));

	// copy title screen over

	int	lineTotal = r_coreInfo.mr_frameInfo1.visibleLines * 5;

	extern uint16_t __attribute__((space(prog))) TitleGraph1[];
	extern uint16_t __attribute__((space(prog))) TitleColor1[];
	extern uint16_t __attribute__((space(prog))) TitleBackColor1[];

	extern uint16_t __attribute__((space(prog))) TitleGraph2[];
	extern uint16_t __attribute__((space(prog))) TitleColor2[];
	extern uint16_t __attribute__((space(prog))) TitleBackColor2[];
    
	copyFromFlash(r_coreInfo.mr_frameInfo1.graphBuf, (uint16_t)TitleGraph1, lineTotal);
	copyFromFlash(r_coreInfo.mr_frameInfo1.colorBuf, (uint16_t)TitleColor1, lineTotal);
	copyFromFlash(r_coreInfo.mr_frameInfo1.colorBKBuf, (uint16_t)TitleBackColor1, r_coreInfo.mr_frameInfo1.visibleLines);

	copyFromFlash(r_coreInfo.mr_frameInfo2.graphBuf, (uint16_t)TitleGraph2, lineTotal);
	copyFromFlash(r_coreInfo.mr_frameInfo2.colorBuf, (uint16_t)TitleColor2, lineTotal);
	copyFromFlash(r_coreInfo.mr_frameInfo2.colorBKBuf, (uint16_t)TitleBackColor2, r_coreInfo.mr_frameInfo2.visibleLines);

}

void
setupDisk()
{
	// now load the file
	while (!pf_mount())
	{
		flash_led(4);
	}

	// first available regular file
	state.io_frameNumber = 1;
	state.io_bits &= ~STATE_PLAYING;
	while (!pf_open_first(&state.i_numFrames))
	{
		flash_led(3);
	}

	SPI1_HighSpeed();
}

void
resetNewCode()
{
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    FLASH_ErasePage((uint16_t)&main2);
	FLASH_WriteDoubleWord24((uint16_t)&main2, 0xFFFFFF, 0xFFFFFF);
    FLASH_Lock();
}

void
handleFirmwareUpdate()
{
	// is this an udpate file?
	if (memcmp(fsInfo.name, "UPDATE  FRM", 11) != 0)
        return;
    
	INTERRUPT_GlobalDisable();

	// first pass write,
	// second pass confirm

	for (uint8_t i=0; i<2; i++)
	{
		if (i == 0)
			FLASH_Unlock(FLASH_UNLOCK_KEY);

		uint32_t	block = 0;	// blocks
		uint32_t	dst = (uint16_t)&main2;
		uint32_t	erased_pc = dst;

		int32_t	size = fsInfo.fsize;
		while(size > 0)
		{
			while (!pf_seek_block(block))
			{
				flash_led(2);
			}

			uint32_t*	buf = (uint32_t*)mr_buffer1;
			pf_read_block(mr_buffer1);


			int32_t	remaining_chunk = 512;

			while (remaining_chunk > 0)
			{
				uint32_t a = buf[0];
				uint32_t b = buf[1];

				// write
				if (i == 0)
				{
					// make some space
					while (erased_pc < (dst + 4))
					{
						if (!FLASH_ErasePage(erased_pc))
							goto bad;
						erased_pc += FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS;	// 2K
					}
					
					if (!FLASH_WriteDoubleWord24(dst, a, b))
						goto bad;
				}
				else // confirm
				{
					if (FLASH_ReadWord24(dst+0) != a)
						goto bad;

					if (FLASH_ReadWord24(dst+2) != b)
						goto bad;
				}

				dst  += 4;
				buf  += 2;
				
				remaining_chunk -= 8;
				size -= 8;
				
				// may happen for short blocks
				if (size <= 0)
					break;

				// overwriting configuration memory?
				if (dst >= 0xAF00)
					goto bad;
			}

			block += 1; // next 512 bytes
		};

		if (i == 0)
			FLASH_Lock();
	}


//good:

	while(1)
	{
		flash_led(0);
		flash_led(2);
		flash_led(2);	// good
	}

bad:

	// back to default
	resetNewCode();

	while(1)
	{
		flash_led(0);
		flash_led(3);
		flash_led(3);	// bad
	}
}

void
prepareNextFrame()
{
	struct frameInfo*	fInfo;

	if (r_coreInfo.mr_bufferIndex)
		fInfo = &r_coreInfo.mr_frameInfo1;
	else
		fInfo = &r_coreInfo.mr_frameInfo2;
	
	uint8_t*	dst = fInfo->buffer;
	uint32_t	offset = (state.io_frameNumber * FIELD_NUM_BLOCKS);

	while (!pf_seek_block(offset))
	{
		flash_led(4);
	}

	// first block
	pf_read_block(dst);
	dst += 512;
	frameInit(fInfo);
	
	// remaining blocks
	int nb = fInfo->numBlocks - 1;
	while(nb)
	{
		pf_read_block(dst);
		dst += 512;
		nb--;
	}
				
	updateBuffer(&state, fInfo);
}

void
waitEndFrame()
{
	// wait for transition
	TESTA2_LOW
	while (!r_coreInfo.mr_endFrame)
	{

	}
	r_coreInfo.mr_endFrame = 0;
	TESTA2_HIGH
}

void
runTitle()
{
	uint16_t		m_titleFrame = 300; // 5 seconds

	// sample the file to see if title should be PAL format etc
	state.io_frameNumber = 1;

	waitEndFrame();
	prepareNextFrame();
	waitEndFrame();
	prepareNextFrame();

	int		prevVis = r_coreInfo.mr_frameInfo1.visibleLines;
   

	// now override title to file height

	setupTitle();

	int		diff = prevVis - r_coreInfo.mr_frameInfo1.visibleLines;

	r_coreInfo.mr_frameInfo1.visibleLines += diff;
	r_coreInfo.mr_frameInfo2.visibleLines += diff;

	r_coreInfo.mr_frameInfo1.totalLines += diff;
	r_coreInfo.mr_frameInfo2.totalLines += diff;

	while(m_titleFrame--)
	{
		waitEndFrame();

		state.i_swcha = r_coreInfo.mr_swcha;
		state.i_swchb = r_coreInfo.mr_swchb;
		state.i_inpt4 = r_coreInfo.mr_inpt4;
		state.i_inpt5 = r_coreInfo.mr_inpt5;

		updateTransport(&state);

		// if reset button pressed skip title frame
		if (state.io_bits & STATE_PLAYING)
			break;
	}

}

void
runFrameLoop()
{
	uint_fast8_t	t = 0;

	state.io_frameNumber = 1;
	state.io_bits =  STATE_PLAYING;

	while(1)
	{
		waitEndFrame();

		t++;
        if (t == 60)
        {
			TESTA0_LOW
            t = 0;
        }
        else
        {
			TESTA0_HIGH
        }
        
		state.i_swcha = r_coreInfo.mr_swcha;
		state.i_swchb = r_coreInfo.mr_swchb;
		state.i_inpt4 = r_coreInfo.mr_inpt4;
		state.i_inpt5 = r_coreInfo.mr_inpt5;

		updateTransport(&state);
		prepareNextFrame();
	}
}


int
main(void)
{    
    // initialize the device
    SYSTEM_Initialize();


    // check for new code
    {
        uint32_t newCode = FLASH_ReadWord24((uint16_t)&main2);    
        if (newCode != 0xFFFFFF && newCode != 0x60000)
        {
			// check for factory reset RB8(PGD) RB9(PGC) have WPU enabled
			if (!_RB8 && _RB9)
			{
				resetNewCode();
				while(1)
				{
					flash_led(4);
					flash_led(4);
				}
			}
			else
			{
				const void* address = &main2 + 2;
				goto *address;
			}
        }
    }
    

    // safe to start (will be in reset several times)
	coreInit();

#if 0 // Use this to test measure startup time
    uint_fast8_t i, j;
    for (i=10;i>0; i--)
    {
        for (int j=10; j>0; j--)
            TESTA2_HIGH
        for (int j=10; j>0; j--)
            TESTA2_LOW    
    }
#endif

	setupTitle();
	setupDisk();
	handleFirmwareUpdate();
	updateInit();
	runTitle();
	runFrameLoop();
}

#include "mcc_generated_files/clc1.h"
#include "sd_reader.h"

extern __attribute__((section(".queueInfo"))) struct queueInfo qinfo;
extern uint16_t ld_word (const uint8_t* ptr);
extern uint32_t ld_dword (const uint8_t* ptr);
extern uint32_t get_fat (uint32_t clst);

int pf_open_patch( uint32_t *numFrames, int which);


// new updates placed here
// must be page_erase aligned
__attribute__((section(".newcode"),space(prog)))
void main2(void)
{
	// add these always since its not clear if we need to jump to main or main+2
    asm("nop");
    asm("nop");

	// March 25 2024
	// 7800 needs to select on A12+A11, (works for FB2 since A12 is always high)

	__builtin_write_RPCON(0x0000); // unlock PPS
	  RPINR46bits.CLCINBR = 0x003C;    //RC12->CLC1:CLCINB
    __builtin_write_RPCON(0x0800); // lock PPS

	CLC1CONL = 0x80A2 & ~(0x8000);
	CLC1CONH = 0x0C;
	CLC1SELL = 0x5000;
	CLC1GLSL = 0x802;
	CLC1GLSH = 0x00;
    CLC1_Enable();

	/*
    9,217    4800          000000       main2      NOP                                           
    9,218    4802          000000                  NOP                                           
    9,219    4804          20FFF0                  MOV #0xFFF, W0                                
    9,220    4806          887260                  MOV W0, CNEN1C                                
    9,221    4808          23F000                  MOV #0x3F00, W0                               
    9,222    480A          887210                  MOV W0, CNPUC                                 
    9,223    480C          07EAEB                  RCALL coreInit                                
    9,224    480E          07EA67                  RCALL setupTitle                              
    9,225    4810          07EF3F                  RCALL setupDisk                               
    9,226    4812          07EFC6                  RCALL handleFirmwareUpdate                    
    9,227    4814          07F09C                  RCALL updateInit                              
    9,228    4816          07EAC0                  RCALL runTitle                                
    9,229    4818          37EF89                  BRA runFrameLoop                              
	*/

	// August 30 2024
	// Add support for 'SELECT' moving to next mvc file

    // same as before
	coreInit();
	setupTitle();
	setupDisk();
	handleFirmwareUpdate();
	updateInit();
	runTitle();

	// frame loop2 with select to next file support

	uint_fast8_t	t = 0;
	int				which = 1;
	uint_fast8_t    lswchb = 0xff;

	state.io_frameNumber = 1;
	state.io_bits =  STATE_PLAYING;

	while(1)
	{
		waitEndFrame();

		t++;
        if (t == 60)
        {
			TESTA0_LOW
            t = 0;
        }
        else
        {
			TESTA0_HIGH
        }
        
		state.i_swcha = r_coreInfo.mr_swcha;
		state.i_swchb = r_coreInfo.mr_swchb;
		state.i_inpt4 = r_coreInfo.mr_inpt4;
		state.i_inpt5 = r_coreInfo.mr_inpt5;

		updateTransport(&state);
		prepareNextFrame();

		// select moves to next video
		if (!(state.i_swchb & 0x02) && (lswchb & 0x02))
		{
			which++;
			while (pf_open_patch(&state.i_numFrames, which) != which)
			{
				which = 1;
			}

			// reset rewind cache
			qinfo.head = 0;
			for (int i=0; i<QUEUE_SIZE; i++)
			{
				qinfo.block[i] = -1;
				qinfo.clust[i] = -1;
			}

			state.io_frameNumber = 1;
			state.io_bits |= STATE_PLAYING;

			// go to black
			memset(mr_buffer1, 0, sizeof(mr_buffer1));
			memset(mr_buffer2, 0, sizeof(mr_buffer2));

			// debounce
			for (int i=0; i<30; i++)
				waitEndFrame();
		}

		lswchb = state.i_swchb;
	}
}

__attribute__((section(".newcode2"),space(prog)))
int pf_open_patch( uint32_t *numFrames, int which)
{
	bool		res = true;
	uint8_t		c;
	uint8_t*	buf;
	uint8_t*	dir;

	uint16_t	dir_index = 0;			// Current read/write index number 
	uint32_t	dir_clust = fsInfo.dirbase;	// Current cluster 
	uint32_t	dir_sect = clust2sect(dir_clust);	// Current sector 

	int			found = 0;

	*numFrames = 0;

	do 
	{
		buf = disk_read_block1(dir_sect);
		dir = &buf[(dir_index & 15) << 5];

		c = dir[DIR_Name];	// First character 

		// Reached to end of table 
		if (c == 0)
		{
			break;
		}

		if (c != 0xe5)	// not deleted
		{
			// regular archive file
			if (dir[DIR_Attr] == AM_ARC)
			{
				// File start cluster 
				uint8_t	*b = (uint8_t*)&fsInfo.org_clust;

				// little endian architecture
				b[3] = dir[DIR_FstClusHI + 1];
				b[2] = dir[DIR_FstClusHI + 0];
				b[1] = dir[DIR_FstClusLO + 1];
				b[0] = dir[DIR_FstClusLO + 0];

				fsInfo.fsize = ld_dword(dir+DIR_FileSize);	// File size 
				*numFrames = fsInfo.fsize / (FIELD_NUM_BLOCKS * 512);

				fsInfo.block = 0;						// File pointer 
				fsInfo.curr_clust = fsInfo.org_clust;

				// store 8.3 name
				memcpy(fsInfo.name, &dir[DIR_Name], 11);

				found++;
				if (found >= which)
					break;
			}
		}

		// Next entry 
		dir_index++;
		if (!dir_index || !dir_sect) // Report EOT when index has reached 65535 
		{
			res = false;	
		}
		else if (!(dir_index & 15)) // Sector changed? 
		{
			dir_sect++;			// Next sector 

			// Static table 
			if (dir_clust == 0)
			{
				if (dir_index >= fsInfo.n_rootdir)
					res = false;	// Report EOT when end of table 
			}
			else // Dynamic table 
			{
				// Cluster changed? 
				if (((dir_index >> 4) & fsInfo.csize_mask) == 0)
				{
					dir_clust = get_fat(dir_clust);		// Get next cluster 
					if (dir_clust >= fsInfo.n_fatent)
						res = false;			// Report EOT when it reached end of dynamic table 
					else
						dir_sect = clust2sect(dir_clust);
				}
			}
		}

	} while (res == true);

	return res ? found : 0;
}





        
/**
 End of File
*/
