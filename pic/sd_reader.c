#if 0

sd_reader.c

Low level SD card access
Bytes are read 512 bytes at a time, using DMA, into one of two frame buffers, over multiple calls, which poll status

#endif


#include "defines.h"
#include "mcc_generated_files/mcc.h"
#include "custom_spi.h"

extern void test_flash(uint8_t num);

#define FRAME_DATA_OFFSET 		4
#define AUDIO_DATA_OFFSET 		7
#define GRAPH_DATA_OFFSET 		269
#define TIMECODE_DATA_OFFSET 	1229
#define COLOR_DATA_OFFSET 		1289
#define END_DATA_OFFSET			2249


// custom, receive 512 bytes as quickly as possible
void read512Start(uint8_t *dst);
bool read512Done(void);

bool sdcard_init(void);

// Don't auto initialize these on startup (too slow), so make persistent!
static __persistent uint8_t         sd_frame1[FIELD_SIZE];
static __persistent uint8_t         sd_frame2[FIELD_SIZE];

uint8_t         *sd_ptr_audio = 0;
uint8_t         *sd_ptr_graph = 0;
uint8_t         *sd_ptr_timecode = 0;
uint8_t         *sd_ptr_color = 0;
uint8_t         *sd_ptr_data = 0;

uint32_t         startBlock = 0;

uint8_t			 state_readState = 2;
uint8_t			*state_readBuf = 0;
uint8_t			 state_remainingBlocks = 0;
bool			 state_needEnd = false;

//////////////////////


bool	error_ = false;
uint8_t status_ = 0;
uint8_t type_ = 0;


#define SD_CARD_ERROR_CMD17 1
#define SD_CARD_ERROR_CMD8  2
#define SD_CARD_ERROR_CMD58 3
#define SD_CARD_ERROR_CMD12 4

#define SD_CARD_ERROR_LOOP1	5
#define SD_CARD_ERROR_LOOP2	6
#define SD_CARD_ERROR_LOOP3	7
#define SD_CARD_ERROR_LOOP4	8


/* Definitions for MMC/SDC command */
#define CMD0     (0)    /* GO_IDLE_STATE */
#define CMD1     (1)    /* SEND_OP_COND */
#define CMD8     (8)    /* SEND_IF_COND */
#define CMD9     (9)    /* SEND_CSD */
#define CMD10    (10)   /* SEND_CID */
#define CMD12    (12)   /* STOP_TRANSMISSION */
#define CMD16    (16)   /* SET_BLOCKLEN */
#define CMD17    (17)   /* READ_SINGLE_BLOCK */
#define CMD18    (18)   /* READ_MULTIPLE_BLOCK */
#define CMD23    (23)   /* SET_BLOCK_COUNT */
#define CMD24    (24)   /* WRITE_BLOCK */
#define CMD25    (25)   /* WRITE_MULTIPLE_BLOCK */
#define CMD41    (41)   /* SEND_OP_COND (ACMD) */
#define CMD55    (55)   /* APP_CMD */
#define CMD58    (58)   /* READ_OCR */

#define ACMD41 0x29

#define SD_CARD_TYPE_SD1  1
#define SD_CARD_TYPE_SD2  2
#define SD_CARD_TYPE_SDHC 3

#define R1_READY_STATE		0x00
#define R1_IDLE_STATE 		0x01
#define R1_ILLEGAL_COMMAND	0x04

#define DATA_START_BLOCK	0xFE


//#define spiRec()  SPI1_Exchange8bit(0xff)
// need more delay
uint8_t
spiRec()
{
    NOP();NOP();//NOP();NOP();NOP();NOP();NOP();NOP();NOP();NOP();
	return SPI1_Exchange8bit(0xff);
}


#define spiSend(X) SPI1_Exchange8bit(X)
#if 0
spiSend(uint8_t X)
{
    NOP();NOP();NOP();NOP();NOP();NOP();NOP();NOP();NOP();NOP();
	return SPI1_Exchange8bit(X);
}
#endif


#define spiInitialize() SPI1_Initialize()
#define spiOpenHighSpeed() SPI1_Open()

void 
chipSelectHigh()
{
	SPI1_SS = 1;
}

void 
chipSelectLow()
{
	SPI1_SS = 0;
}

void
sd_error(uint8_t type)
{
	chipSelectHigh();
//	test_flash(type);
	error_ = true;
}

// send command and return error code.  Return zero for OK
uint8_t 
cardCommand(uint8_t cmd, uint32_t arg) 
{
	chipSelectLow();


	uint8_t	cnt = 255;

	// wait not busy
	while(cnt--)
	{
		if (spiRec() == 0xFF)
			break;
	};

	if (!cnt)
	{
		sd_error(SD_CARD_ERROR_LOOP1);
		return 0xFF;
	}

	// send command
	spiSend(cmd | 0x40);

	// send argument
	for (int8_t s = 24; s >= 0; s -= 8) 
		spiSend(arg >> s);

	// send CRC
	uint8_t crc = 0XFF;
	if (cmd == CMD0)
		crc = 0X95;  // correct crc for CMD0 with arg 0
	if (cmd == CMD8)
		crc = 0X87;  // correct crc for CMD8 with arg 0X1AA
	spiSend(crc);

	if (cmd == CMD12)
		spiRec();   /* Skip a stuff byte when stop reading */

	// wait for response

	cnt = 255;
	while(cnt--)
	{
		status_ = spiRec();
		if (!(status_ & 0x80))
			break;
	};

	if (!cnt)
	{
		sd_error(SD_CARD_ERROR_LOOP2);
		return 0xFF;
	}

	return status_;
}

/** Wait for start block token */
bool
startBlockReady(void) 
{
	if (spiRec() == DATA_START_BLOCK)
		return true;

	return false;
}


void
startReadBlock(uint32_t block)
{
	// use address if not SDHC card
	if (type_ != SD_CARD_TYPE_SDHC)
		block <<= 9;

	if (cardCommand(CMD17, block)) 
	{
		sd_error(SD_CARD_ERROR_CMD17);
	}
}

void
startReadBlockMulti(uint32_t block)
{
	// use address if not SDHC card
	if (type_ != SD_CARD_TYPE_SDHC)
		block <<= 9;

	if (cardCommand(CMD18, block)) 
	{
		sd_error(SD_CARD_ERROR_CMD12);
	}
}

void
endReadBlockMulti()
{
	if (cardCommand(CMD12, 0)) 
	{
		sd_error(SD_CARD_ERROR_CMD17);
	}
}

void
readBlockEnd() 
{
	// read rest of data, checksum and set chip select high
	// skip data and crc
	spiRec();
	spiRec();
	chipSelectHigh();
}


uint8_t 
cardAcmd(uint8_t cmd, uint32_t arg) 
{
	cardCommand(CMD55, 0);
	return cardCommand(cmd, arg);
}

////////////////////////////////////////////////////////////////////////////////


void
sd_runReadState()
{
	switch(state_readState)
	{
		case 0:
			if (!startBlockReady())
				return;
			read512Start(state_readBuf);
			state_readBuf += BLOCK_SIZE;
			state_readState = 1;
			state_remainingBlocks--;
			// fallthrough

		case 1:
			if (!read512Done())
				return;
			if (state_remainingBlocks)
				state_readState = 0;
			else
				state_readState = 2;
			return;

		case 2:
			return;             
	}
}

void
sd_swapField(bool index)
{
	if (index == true)
	{
		sd_ptr_data  = sd_frame1;
		sd_ptr_audio = sd_frame1 + AUDIO_DATA_OFFSET;
		sd_ptr_graph = sd_frame1 + GRAPH_DATA_OFFSET;
		sd_ptr_timecode = sd_frame1 + TIMECODE_DATA_OFFSET;
		sd_ptr_color = sd_frame1 + COLOR_DATA_OFFSET;
	}
	else
	{
		sd_ptr_data  = sd_frame2;
		sd_ptr_audio = sd_frame2 + AUDIO_DATA_OFFSET;
		sd_ptr_graph = sd_frame2 + GRAPH_DATA_OFFSET;
		sd_ptr_timecode = sd_frame2 + TIMECODE_DATA_OFFSET;
		sd_ptr_color = sd_frame2 + COLOR_DATA_OFFSET;
	}
}

bool
sd_openStream() 
{		
	// see if the card is present and can be initialized:
	while (!sdcard_init()) 
	{
		test_flash(1);
		//		return false;
	}

	spiOpenHighSpeed();

	uint32_t    block = 0;

#if 1  // slow..
	// check every 32k, sector
	while(1)
	{
		startReadBlock(block);
		while(!startBlockReady())
		{
		}

		read512Start(sd_frame1);
		while(!read512Done())
		{
		}

		readBlockEnd();

		// look for frame 1
		if ((sd_frame1[0] == 0 && sd_frame1[1] == 0 && sd_frame1[2] == 1))
		{
			startBlock = block;
			break;
		}


		// jump to start of each 32k sector
		block += (32768 / 512);
		//       block++;
	}
#endif

	//    startBlock = (1024 * 512) - 1;
	//    startBlock = 0x487; // 1159 = 1024 + 135 ? works Jan 2020

	return true;
}

void
sd_readField(uint32_t fr, bool index)
{
	if (error_)
	{
		sdcard_init();
		return;
	}

	//   uint32_t     block = startBlock + (fr * NUM_BLOCKS);
	uint32_t     block = startBlock + (fr * FULL_NUM_BLOCKS);

	if (index == true)
		state_readBuf = sd_frame1;
	else
		state_readBuf = sd_frame2;

	if (state_needEnd)
	{
		readBlockEnd();
		endReadBlockMulti();
		state_needEnd = false;
	}

	startReadBlockMulti(block);

	state_remainingBlocks = NUM_BLOCKS;
	state_readState = 0;

	//    while(state_remainingBlocks > 1)
	{
		sd_runReadState();
	}

	// rest will be feed through on more state calls

	state_needEnd = true;
}

bool
sdcard_init() 
{
	uint8_t		cnt;
	error_ = type_ = 0;

	spiInitialize();

	chipSelectHigh();

	// must supply min of 74 clock cycles with CS high.
	for (uint8_t i = 0; i < 10; i++)
		spiSend(0XFF);

	chipSelectLow();


	// command to go idle in SPI mode
	cnt = 255;
	while(cnt--)
	{
		status_ = cardCommand(CMD0, 0);
		if (error_)
			break;
		if (status_ == R1_IDLE_STATE)
			break;    
	}

	if (!cnt)
	{
		sd_error(SD_CARD_ERROR_LOOP3);
		return false;
	}

	// check SD version
	if ((cardCommand(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) 
	{
		type_ = SD_CARD_TYPE_SD1;
	}
	else 
	{
		// only need last byte of r7 response
		for (uint8_t i = 0; i < 4; i++) 
			status_ = spiRec();

		if (status_ != 0XAA) 
		{
			sd_error(SD_CARD_ERROR_CMD8);
			return false;
		}
		type_ = SD_CARD_TYPE_SD2;
	}

	// initialize card and send host supports SDHC if SD2

	uint32_t arg = (type_ == SD_CARD_TYPE_SD2) ? 0X40000000 : 0;


	cnt = 255;
	while(cnt--)
	{
		status_ = cardAcmd(ACMD41, arg);
		if (error_)
			break;
		if (status_ == R1_READY_STATE)
			break;
	}

	if (!cnt)
	{
		sd_error(SD_CARD_ERROR_LOOP4);
		return false;
	}

	// if SD2 read OCR register to check for SDHC card
	if (type_ == SD_CARD_TYPE_SD2) 
	{
		if (cardCommand(CMD58, 0)) 
		{
			sd_error(SD_CARD_ERROR_CMD58);
			return false;
		}

		if ((spiRec() & 0XC0) == 0XC0)
			type_ = SD_CARD_TYPE_SDHC;
		// discard rest of ocr - contains allowed voltage range
		for (uint8_t i = 0; i < 3; i++)
			spiRec();
	}
	chipSelectHigh();

	return true;

}

void
read512Start(uint8_t *dst)
{
	uint8_t xmit_byte = 0xff;

	SPI1TCNT = 512;     // why is this needed?

	/* stop and clear DMA 1  */
	DMA1CON0 = 0;
	/* stop and clear DMA 2  */
	DMA2CON0 = 0;


	PIR2bits.DMA1DCNTIF =0; // clear Destination Count Interrupt Flag bit
	PIR2bits.DMA1SCNTIF =0; // clear Source Count Interrupt Flag bit
	PIR2bits.DMA1AIF =0; // clear abort Interrupt Flag bit
	PIR2bits.DMA1ORIF =0; // clear overrun Interrupt Flag bit

	PIE2bits.DMA1DCNTIE =0; // disable Destination Count 0 Interrupt
	PIE2bits.DMA1SCNTIE =0; // disable Source Count Interrupt
	PIE2bits.DMA1AIE =0; // disable abort Interrupt
	PIE2bits.DMA1ORIE =0; // disable overrun Interrupt 


	PIR5bits.DMA2DCNTIF =0; // clear Destination Count Interrupt Flag bit
	PIR5bits.DMA2SCNTIF =0; // clear Source Count Interrupt Flag bit
	PIR5bits.DMA2AIF =0; // clear abort Interrupt Flag bit
	PIR5bits.DMA2ORIF =0; // clear overrun Interrupt Flag bit

	PIE5bits.DMA2DCNTIE =0; // disable Destination Count 0 Interrupt
	PIE5bits.DMA2SCNTIE =0; // disable Source Count Interrupt
	PIE5bits.DMA2AIE =0; // disable abort Interrupt
	PIE5bits.DMA2ORIE =0; // disable overrun Interrupt 


	// DMA1:  [0xff] -> SPI1TX  512 times
	DMA1SSA = (uint16_t)&xmit_byte;  // source start address
	DMA1CON1bits.SMR	= 0;    // SFR/GPR space as source memory
	DMA1CON1bits.SMODE	= 0;    // source address fixed
	DMA1SSZ				= 512;	// source size
	DMA1CON1bits.SSTP	= 1;    // SIRQEN cleared after sending 8 bytes

	DMA1DSA = (uint16_t)&SPI1TXB;      // 0x3D11 dest start address SPI1TXB
	DMA1CON1bits.DMODE	= 0;    // destination address fixed
	DMA1DSZ				= 1;    // destination size 1 byte
	DMA1CON1bits.DSTP	= 0;    // No destination reload stop bit

	DMA1SIRQ			= 21;	// SPI1TX as Transfer Trigger Source
	DMA1AIRQ			= 0;	// None


	// DMA2:  SPIRX  -> [dst++] 512 times

	DMA2SSA = (uint16_t)&SPI1RXB;     // 0x3D10 source address SPI1RXB
	DMA2CON1bits.SMR	= 0;    // SFR/GPR space as source memory
	DMA2CON1bits.SMODE	= 0;    // source address fixed
	DMA2SSZ 			= 1;	// 1 or 512??
	DMA2CON1bits.SSTP	= 0;    // No source reload stop bit

	DMA2DSA = (uint16_t)dst;	// dest address 
	DMA2CON1bits.DMODE	= 1;    // destination address increment
	DMA2DSZ				= 512;	// dest size
	DMA2CON1bits.DSTP	= 1;	// SIRQEN cleared after rcving 8 bytes

	DMA2SIRQ			= 20;	// SPI1RX as Transfer Trigger Source
	DMA2AIRQ			= 0;	// None


	/* unlock Arbiter settings */
#if 0
    // may introduces extra banksel write statement which breaks timing
    PRLOCK = 0x55;
    PRLOCK = 0xAA;
    PRLOCKbits.PRLOCKED = 0;
#endif
        
    asm("BANKSEL PRLOCK");
	asm("MOVLW 0x55");
	asm("MOVWF PRLOCK");
	asm("MOVLW 0xAA");
	asm("MOVWF PRLOCK");
	asm("BCF PRLOCK, 0");

	MAINPR = 3;
	ISRPR = 2;
	DMA1PR = 0;
	DMA2PR = 1;
	SCANPR = 4;

	/* lock Arbiter settings */
#if 0
    // may introduces extra banksel write statement which breaks timing
    PRLOCK = 0x55;
    PRLOCK = 0xAA;
    PRLOCKbits.PRLOCKED = 1;
#endif
        
    asm("BANKSEL PRLOCK");
	asm("MOVLW 0x55");
	asm("MOVWF PRLOCK");
	asm("MOVLW 0xAA");
	asm("MOVWF PRLOCK");
	asm("BSF PRLOCK, 0");

	// enable
	DMA1CON0bits.DMA1EN = 1;
	DMA2CON0bits.DMA2EN = 1;

	///////

	DMA1CON0bits.DMA1SIRQEN = 1; // Source Trigger starts DMA transfer
	DMA2CON0bits.DMA2SIRQEN = 1; // Dest Trigger starts DMA transfer


	// todo:  last byte may not be transferred?
	// check: SPI1INTFbits.SRMTIF, SPI1INTFbits.TCZIF?
}

bool
read512Done()
{
	if (SPI1TCNT)
		return false;

	if (SPI1CON2bits.BUSY)
		return false;

	return true;
}

