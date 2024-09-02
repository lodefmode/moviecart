/*----------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module  R0.03a
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2019, ChaN, all right reserved.
/
/ Petit FatFs module is an open source software. Redistribution and use of
/ Petit FatFs in source and binary forms, with or without modification, are
/ permitted provided that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/-----------------------------------------------------------------------------/
/ Jun 15,'09  R0.01a  First release.
/
/ Dec 14,'09  R0.02   Added multiple code page support.
/                     Added write funciton.
/                     Changed stream read mode interface.
/ Dec 07,'10  R0.02a  Added some configuration options.
/                     Fixed fails to open objects with DBCS character.

/ Jun 10,'14  R0.03   Separated out configuration options to pffconf.h.
/                     Added _USE_LCC option.
/                     Added _FS_FAT16 option.
/
/ Jan 30,'19  R0.03a  Supported stdint.h for C99 and later.
/                     Removed _WORD_ACCESS option.
/                     Changed prefix of configuration options, _ to PF_.
/                     Added some code pages.
/                     Removed some code pages actually not valid.
/----------------------------------------------------------------------------*/

// Reduced greatly for moviecart application -> Rob Bairos 2021-2023

#include <string.h> // memcpy
#include "pff.h"		// Petit FatFs configurations and declarations 
#include "sd_reader.h"	// Declarations of low level disk I/O functions 
#include "core.h"


//--------------------------------------------------------------------------
//   Module Private Definitions
//---------------------------------------------------------------------------

// FatFs refers the members in the FAT structures with byte offset instead
// of structure member because there are incompatibility of the packing option
// between various compilers. 

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BPB_BytsPerSec		11
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BS_55AA				510

#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54

#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82

#define MBR_Table			446



__attribute__((section(".fileSystemInfo"))) struct fileSystemInfo	fsInfo;

//-----------------------------------------------------------------------
// Load multi-byte word in the FAT structure                             
//-----------------------------------------------------------------------

// little endian architecture
#if 0	// doesn't work, do src + dst need to be word aligned?
#define ld_word(X) ( *((uint16_t*)(X)) )
#define ld_dword(X) ( *((uint32_t*)(X)) )
#endif

uint16_t ld_word (const uint8_t* ptr)   /*   Load a 2-byte little-endian word */
{
    uint16_t rv;

    rv = ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

uint32_t ld_dword (const uint8_t* ptr) /* Load a 4-byte little-endian word */
{
    uint32_t rv;

    rv = ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];

    return rv;
}


//-----------------------------------------------------------------------
// FAT access - Read value of a FAT entry                                
//-----------------------------------------------------------------------

uint32_t get_fat (
	uint32_t clst	// Cluster# to get the link information 
)
{
	uint8_t*	buf = disk_read_block1(fsInfo.fatbase + (clst >> 7));
	uint16_t	offset =  (((uint16_t)clst & 127) << 2);

	return ld_dword(&buf[offset]) & 0x0FFFFFFF;
}

//-----------------------------------------------------------------------
// Mount/Unmount a Locical Drive                                         
//-----------------------------------------------------------------------


// ring buffer..

__attribute__((section(".queueInfo"))) struct queueInfo qinfo;


bool pf_mount()
{
	uint8_t* buf;
	uint32_t bsect, fsize, tsect;

	// Check if the drive is ready or not 
	if (!disk_initialize())
		return false;

	qinfo.head = 0;
	for (int i=0; i<QUEUE_SIZE; i++)
	{
		qinfo.block[i] = -1;
		qinfo.clust[i] = -1;
	}

	// Search FAT partition on the drive 

	bsect = 0;

	while(1)
	{
		buf = disk_read_block1(bsect);	// Read the boot record 

		// Check record signature 
		if (ld_word(&buf[510]) != 0xAA55)
			return false;	// not a boot record, error

		// Check FAT32 
		if (ld_word(&buf[BS_FilSysType32]) == 0x4146)
			break;	// valid

		// Not an FAT boot record, it may be FDISK format 
		// Check a partition listed in top of the partition table 

		buf += MBR_Table;

		// Is the partition existing? 
		if (buf[4])
		{
			bsect = ld_dword(&buf[8]);	// Partition offset in LBA 
			continue;
		}

		return false;	// not a boot record, error
	}


	// Initialize the file system object 

	fsize = ld_word(buf+BPB_FATSz16);				// Number of sectors per FAT 
	if (!fsize)
		fsize = ld_dword(buf+BPB_FATSz32);

	fsize *= buf[BPB_NumFATs];						// Number of sectors in FAT area 
	fsInfo.fatbase = bsect + ld_word(buf+BPB_RsvdSecCnt); // FAT start sector (lba) 
	fsInfo.n_rootdir = ld_word(buf+BPB_RootEntCnt);		// Nmuber of root directory entries 


	// Number of sectors per cluster 
	
	fsInfo.csize = buf[BPB_SecPerClus];
	fsInfo.csize_bits = -1;

	while(fsInfo.csize)
	{
		fsInfo.csize_bits++;
		fsInfo.csize >>= 1;
	}

	fsInfo.csize = 1 << fsInfo.csize_bits;
	fsInfo.csize_mask = fsInfo.csize - 1;


	tsect = ld_word(buf+BPB_TotSec16);				// Number of sectors on the file system 
	if (!tsect)
		tsect = ld_dword(buf+BPB_TotSec32);

	// Last cluster# + 1 
	fsInfo.n_fatent = ((tsect - ld_word(buf+BPB_RsvdSecCnt) - fsize - (fsInfo.n_rootdir >> 4)) >> fsInfo.csize_bits) + 2;

	fsInfo.dirbase = ld_dword(buf+(BPB_RootClus));	// Root directory start cluster 
	fsInfo.database = fsInfo.fatbase + fsize + (fsInfo.n_rootdir >> 4);	// Data start sector (lba) 

	// FS_FAT32;
	if (fsInfo.n_fatent >= 0xFFF7)
		return true;

	return false;
}

//-----------------------------------------------------------------------
// Seek File R/W Pointer                                                 
//-----------------------------------------------------------------------


bool pf_seek_block (
	uint32_t block		// File pointer from top of file 
)
{
	uint32_t	sect;
	uint8_t		cs;
	uint8_t		skip = 0;

	// When seek to same or following cluster, start from the current cluster 
	// When seek to back cluster,  start from the first cluster 

	// check previous entries as well


	uint8_t		check;
	uint8_t		i;

	check = qinfo.head - 1;
	check &= QUEUE_MASK;

	for (i=0; i<QUEUE_SIZE; i++)
	{
		if (block >= qinfo.block[check])
		{
			fsInfo.block = qinfo.block[check];
			fsInfo.curr_clust = qinfo.clust[check];
			break;
		}

		check--;
		check &= QUEUE_MASK;
	}

	if (fsInfo.block > block)
	{
		fsInfo.block = 0;
		fsInfo.curr_clust = fsInfo.org_clust;
	}

	// Cluster following loop 
	// Follow cluster chain 


	while (block >= (fsInfo.block + fsInfo.csize))
	{
		skip++;

		// DEFRAG
//		skip &= 127; //  94
//		skip &=  63; // 109
//		skip &=  31; // 136
//		skip &=  15; // 192
//		skip &=   7; // 296 
				
		// shotgun frag:
//		skip &= 127; // 116892
//		skip &= 63;  // 61146
//		skip &= 31;  // 35670
//		skip &= 15;  // 27583
//		skip &=  7;  // 33273

		// Full cache Defrag:   fwd: 18    bck:  18
		// Full cache Shotgun:  fwd: 1830  bck:  1830
		
		// compromise
		// 9136 block reads for just data, so about 5x more reads to reverse if shot-gunned
		// may get slower for larger files

//		skip &=  15;
//		skip &=  31;	// stutter every 9 minutes of rewinding
		skip &=  63;	// stutter every 19 minutes of rewinding on formatted card

		if (!skip)
		{
			qinfo.clust[qinfo.head] =  fsInfo.curr_clust;
			qinfo.block[qinfo.head] =  fsInfo.block;
			qinfo.head++;
			qinfo.head &= QUEUE_MASK;
		}

		fsInfo.curr_clust = get_fat(fsInfo.curr_clust);
		fsInfo.block += fsInfo.csize;
	}


	// Update start block for this cluster 

	sect = clust2sect(fsInfo.curr_clust);		// Get current sector 
	cs = (uint8_t)(block & fsInfo.csize_mask);	// Sector offset in the cluster 

	fsInfo.file_block = sect + cs;

	return true;
}

//-----------------------------------------------------------------------
// First regular non-deleted file										 
//-----------------------------------------------------------------------

bool pf_open_first(
		uint32_t *numFrames
		)
{
	bool		res = true;
	uint8_t		c;
	uint8_t*	buf;
	uint8_t*	dir;

	uint16_t	dir_index = 0;			// Current read/write index number 
	uint32_t	dir_clust = fsInfo.dirbase;	// Current cluster 
	uint32_t	dir_sect = clust2sect(dir_clust);	// Current sector 

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

	return res;
}


void
pf_read_block(uint8_t *dst)
{
	disk_read_block2(fsInfo.file_block++, dst);
}

