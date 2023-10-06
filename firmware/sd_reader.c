#if 0

sd_reader.c

Low level SD card access
Bytes are read 512 bytes at a time, using DMA, into one of two frame buffers, over multiple calls, which poll status

#endif


#include "core.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/spi1.h"

extern void flash_led(uint8_t num);



//////////////////////



struct diskInfo
{
	bool	sd_errorCode;
	uint8_t sd_type;
	uint8_t	sd_status;

	uint32_t sector1;
	uint32_t sector2;

};
__attribute__((section(".diskInfo"))) struct diskInfo	dinfo;
__attribute__((section(".diskBuffer"))) uint8_t	 diskBuffer[512];

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
    Nop();Nop();//Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();
	return SPI1_Exchange8bit(0xff);
}


#define spiSend(X) SPI1_Exchange8bit(X)
#if 0
spiSend(uint8_t X)
{
    Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();
	return SPI1_Exchange8bit(X);
}
#endif


#define spiInitialize() SPI1_Initialize()
#define spiOpenHighSpeed() SPI1_Open()

void 
chipSelectHigh()
{
	IO_RA4_SetHigh();
}

void 
chipSelectLow()
{
	IO_RA4_SetLow();
}

void
sd_error(uint8_t type)
{
	chipSelectHigh();
//	flash_led(type);
	dinfo.sd_errorCode = true;
}

// send command and return error code.  Return zero for OK
uint8_t 
cardCommand(uint8_t cmd, uint32_t arg) 
{
	chipSelectLow();

	uint8_t	cnt = 255;

	dinfo.sd_status = 0;

	// wait not busy
	while(--cnt)
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
	while(--cnt)
	{
		dinfo.sd_status = spiRec();
		if (!(dinfo.sd_status & 0x80))
			break;
	};

	if (!cnt)
	{
		sd_error(SD_CARD_ERROR_LOOP2);
		return 0xFF;
	}

	return dinfo.sd_status;
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
	if (dinfo.sd_type != SD_CARD_TYPE_SDHC)
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
	if (dinfo.sd_type != SD_CARD_TYPE_SDHC)
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
endReadBlock() 
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

bool
sdcard_init() 
{
	uint8_t		cnt;

	dinfo.sd_status = 0;
	dinfo.sd_errorCode = false;
	dinfo.sd_type = 0;

	spiInitialize();

	chipSelectHigh();

	// must supply min of 74 clock cycles with CS high.
	for (uint8_t i = 0; i < 10; i++)
		spiSend(0XFF);

	chipSelectLow();


	// command to go idle in SPI mode
	cnt = 255;
	while(--cnt)
	{
		dinfo.sd_status = cardCommand(CMD0, 0);
		if (dinfo.sd_errorCode)
			return false;
		if (dinfo.sd_status == R1_IDLE_STATE)
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
		dinfo.sd_type = SD_CARD_TYPE_SD1;
	}
	else 
	{
		// only need last byte of r7 response
		for (uint8_t i = 0; i < 4; i++) 
			dinfo.sd_status = spiRec();

		if (dinfo.sd_status != 0XAA) 
		{
			sd_error(SD_CARD_ERROR_CMD8);
			return false;
		}
		dinfo.sd_type = SD_CARD_TYPE_SD2;
	}

	// initialize card and send host supports SDHC if SD2

	uint32_t arg = (dinfo.sd_type == SD_CARD_TYPE_SD2) ? 0X40000000 : 0;


	cnt = 255;
	while(--cnt)
	{
		dinfo.sd_status = cardAcmd(ACMD41, arg);
		if (dinfo.sd_errorCode)
			break;
		if (dinfo.sd_status == R1_READY_STATE)
			break;
	}

	if (!cnt)
	{
		sd_error(SD_CARD_ERROR_LOOP4);
		return false;
	}

	// if SD2 read OCR register to check for SDHC card
	if (dinfo.sd_type == SD_CARD_TYPE_SD2) 
	{
		if (cardCommand(CMD58, 0)) 
		{
			sd_error(SD_CARD_ERROR_CMD58);
			return false;
		}

		if ((spiRec() & 0XC0) == 0XC0)
			dinfo.sd_type = SD_CARD_TYPE_SDHC;
		// discard rest of ocr - contains allowed voltage range
		for (uint8_t i = 0; i < 3; i++)
			spiRec();
	}
	chipSelectHigh();

	return true;

}


/*-----------------------------------------------------------------------*/


static bool disk_read_block_raw (
	uint32_t sector,	/* Sector number (LBA) */
	uint8_t* buf
)
{
	startReadBlock(sector);

	if (dinfo.sd_errorCode)
	{
		while(1)
		{
			flash_led(5);
		};
	};

	while (!startBlockReady())
	{
	}

//	for (i=0; i<512; i++)
//		*buf++ = SPI1_Exchange8bit(0xff);

    SPI1_Exchange8bitBuffer(NULL, 512, buf);

    endReadBlock();

	return true;

}


uint8_t* disk_read_block1 (
	uint32_t sector	/* Sector number (LBA) */
)
{
	if (sector != dinfo.sector1)
	{
		dinfo.sector1 = sector;
		disk_read_block_raw(dinfo.sector1, diskBuffer);
	}

	return diskBuffer;
}



void disk_read_block2 (
	uint32_t sector,	/* Sector number (LBA) */
	uint8_t *dst
)
{
	if (sector != dinfo.sector2)
	{
		dinfo.sector2 = sector;
		disk_read_block_raw(dinfo.sector2, dst);
	}
}

bool
disk_initialize()
{
	dinfo.sector1 = -1;
	dinfo.sector2 = -1;

    // see if the card is present and can be initialized:
    if (!sdcard_init())
		return false;

	if (dinfo.sd_errorCode)
		return false;

	return true;
}

