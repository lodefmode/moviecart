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


#if 1
#include "mcc_generated_files/clock.h"
#define FOSC    (CLOCK_SystemFrequencyGet())
#define FCY     (CLOCK_InstructionFrequencyGet())
#include <libpic30.h>
#endif


uint16_t		m_titleFrame = 300; // 5 seconds


void flash_led(uint8_t num);

#include "pff.h"
#include "core.h"
#include "update.h"

#include "TitleScreen.h"

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



extern uint_fast8_t	mr_swcha;
extern uint_fast8_t	mr_swchb;
extern uint_fast8_t	mr_inpt4;
extern uint_fast8_t	mr_inpt5;	// not used
extern bool			mr_bufferIndex;  // which buffer to fill


extern volatile bool            mr_endFrame;    // set to true at the end of each frame

struct stateVars		state;

// old format
const uint8_t	header1[7] = { 'M', 'V', 'C', 0,  0, 0, 0 };
const uint8_t	header2[7] = { 'M', 'V', 'C', 0,  0, 0, 1 };

uint8_t RAM_UNITIALIZED mr_buffer1[FIELD_SIZE];
uint8_t RAM_UNITIALIZED mr_buffer2[FIELD_SIZE];

struct FrameFormat
{
	uint8_t version[4];   // ('M', 'V', 'C', 0)
	uint8_t format;       // ( 1-------)
	uint8_t timecode[4];  // (hour, minute, second, fame)
	uint8_t vsync;        // eg 3
	uint8_t vblank;       // eg 37
	uint8_t overscan;     // eg 30
	uint8_t visible;      // eg 192
	uint8_t rate;         // eg 60
    
//    uint8_t dataStart;

	// sound[vsync+blank+overscan+visible]
	// graph[5 * visible]
	// color[5 * visible]
	// bkcolor[1 * visible]
	// timecode[60]
	// padding
};

void
initFrameInfo(struct frameInfo* fInfo, uint8_t* dst)
{
	struct 		FrameFormat* ff = (struct FrameFormat* )dst;
	uint16_t	headerSize;

	if (ff->format & 0x80)
	{
		headerSize = sizeof(*ff);

		fInfo->vsyncLines = ff->vsync;
		fInfo->blankLines = ff->vblank;
		fInfo->overscanLines = ff->overscan;
		fInfo->visibleLines = ff->visible;
        fInfo->odd = !(ff->timecode[3] & 1);
		fInfo->totalLines = fInfo->vsyncLines + fInfo->blankLines + fInfo->overscanLines + fInfo->visibleLines;

        fInfo->audioBuf    = dst + headerSize;        
		fInfo->graphBuf    = fInfo->audioBuf + fInfo->totalLines;
		fInfo->colorBuf    = ((uint8_t*)fInfo->graphBuf) + 5 * fInfo->visibleLines;
		fInfo->colorBKBuf  = fInfo->colorBuf + 5 * fInfo->visibleLines;
		fInfo->timecodeBuf = fInfo->colorBKBuf + 1 * fInfo->visibleLines;
	}
	else
	{
		// old format
		// 'M', 'V', 'C', 0, f2, f1, f0
		headerSize = (4 + 3);

		fInfo->vsyncLines = 3;
		fInfo->blankLines = 37;
		fInfo->overscanLines = 30;
		fInfo->visibleLines = 192;
		fInfo->odd = (dst[4 + 3 -1] & 1);
		fInfo->totalLines = fInfo->vsyncLines + fInfo->blankLines + fInfo->overscanLines + fInfo->visibleLines;

		fInfo->audioBuf    = dst + headerSize;
		fInfo->graphBuf    = fInfo->audioBuf + fInfo->totalLines;
		fInfo->timecodeBuf = ((uint8_t*)fInfo->graphBuf) + 5*fInfo->visibleLines;
		fInfo->colorBuf    = ((uint8_t*)fInfo->timecodeBuf) + 60;
		fInfo->colorBKBuf  = fInfo->colorBuf + 5*fInfo->visibleLines;
	}

	if (fInfo->odd)
		fInfo->colorBKBuf++;


	// Calculate total blocks needed

	// sound[vsync+blank+overscan+visible]
	// graph[5 * visible]
	// color[5 * visible]
	// bkcolor[1 * visible]
	// timecode[60]
	// padding

	uint16_t		totalSize = fInfo->totalLines + fInfo->visibleLines*(5 + 5 + 1) + (5*12) + headerSize;

	fInfo->numBlocks = (totalSize >> 9);	// 512 block chunks
	if (totalSize & (512-1))
		fInfo->numBlocks++;
}


int
main(void)
{
    // initialize the device
    SYSTEM_Initialize();

	memcpy(mr_buffer1 , header1, sizeof(header1));
	initFrameInfo(&r_frameInfo, mr_buffer1);

	// safe to start
	coreInit();

	initFrameInfo(&mr_frameInfo1, mr_buffer1);
	memcpy(mr_buffer2 , header2, sizeof(header2));
	initFrameInfo(&mr_frameInfo2, mr_buffer2);

	// copy title screen over, headers first in case they're accessed

	int	lineTotal = mr_frameInfo1.visibleLines * 5;
	
	memcpy(mr_frameInfo1.graphBuf, TitleGraph1, lineTotal);
	memcpy(mr_frameInfo1.colorBuf, TitleColor1, lineTotal);
	memcpy(mr_frameInfo1.colorBKBuf, TitleBackColor1, mr_frameInfo1.visibleLines);
	memset(mr_frameInfo1.audioBuf, 0, mr_frameInfo1.totalLines);

	memcpy(mr_frameInfo2.graphBuf, TitleGraph2, lineTotal);
	memcpy(mr_frameInfo2.colorBuf, TitleColor2, lineTotal);
	memcpy(mr_frameInfo2.colorBKBuf, TitleBackColor2, mr_frameInfo2.visibleLines);
	memset(mr_frameInfo2.audioBuf, 0, mr_frameInfo2.totalLines);
            

	// now load the file
	while (!pf_mount())
	{
		flash_led(3);
	}

	// first available regular file
	state.io_frameNumber = 1;
	state.io_playing = false;
	while (!pf_open_first(&state.i_numFrames, &state.i_numFramesInit))
	{
		flash_led(4);
	}

	SPI1_HighSpeed();


	uint_fast8_t	t = 0;

	while(1)
	{

		// wait for transition
        TESTA2_LOW
		while (!mr_endFrame)
		{

		}
		mr_endFrame = 0;
		TESTA2_HIGH

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
        
		state.i_swcha = mr_swcha;
		state.i_swchb = mr_swchb;
		state.i_inpt4 = mr_inpt4;
		state.i_inpt5 = mr_inpt5;

		updateTransport(&state);

		if (m_titleFrame)
		{
			// if button,reset pressed skip title frame
			if (state.io_playing)
				m_titleFrame = 0;
			else
				m_titleFrame--;

			if (m_titleFrame == 0)
			{
				state.io_frameNumber = 1;
				state.io_playing = 1;
			}
		}
		else
		{
			uint8_t*	dst;
			struct frameInfo*	fInfo;

			if (mr_bufferIndex)
			{
				dst = mr_buffer1;
				fInfo = &mr_frameInfo1;
			}
			else
			{
				dst = mr_buffer2;
				fInfo = &mr_frameInfo2;
			}
			
			uint8_t*	dst0 = dst;
			uint32_t	offset = (state.io_frameNumber * FIELD_NUM_BLOCKS);

			while (!pf_seek_block(offset))
			{
				flash_led(4);
			}

            // first block
            pf_read_block(dst);
			dst += 512;
            initFrameInfo(fInfo, dst0);
            
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
	}

    return 1; 
}

/**
 End of File
*/

