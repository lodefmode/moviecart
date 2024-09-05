
/**
  Section: Included Files
*/

#include <string.h> // memset

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/spi1.h"
#include "mcc_generated_files/memory/flash.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "mcc_generated_files/clc1.h"

#include <libpic30.h>


//#define MVCPATCH 1

__attribute__((section(".patchsection"),space(prog))) void patchLoop(void);

void flash_led(uint8_t num);

#include "pff.h"
#include "core.h"
#include "update.h"
#include "sd_reader.h"

extern struct coreInfo			r_coreInfo;
extern struct fileSystemInfo	fsInfo;
extern struct queueInfo 		qinfo;
extern struct stateVars     	state;

extern uint8_t mr_buffer1[FIELD_SIZE];
extern uint8_t mr_buffer2[FIELD_SIZE];


extern void updateInit();
extern void setupTitle();
extern void setupDisk();
extern void handleFirmwareUpdate();
extern void runTitle();
extern void waitEndFrame();
extern void prepareNextFrame();

int			pf_open_patch( uint32_t *numFrames, int which);

#if 1
__attribute__((section(".patchsection"),space(prog))) void runPatch(void)
{
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

__attribute__((section(".patchsection"),space(prog))) int pf_open_patch( uint32_t *numFrames, int which)
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
#endif

