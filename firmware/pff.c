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

// Reduced greatly for moviecart application -> Rob Bairos 2021-2022

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

#define	DIR_Name			0
#define	DIR_Attr			11
#define	DIR_NTres			12
#define	DIR_CrtTime			14
#define	DIR_CrtDate			16
#define	DIR_FstClusHI		20
#define	DIR_WrtTime			22
#define	DIR_WrtDate			24
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28


// File attribute bits for directory entry 

#define	AM_RDO	0x01	// Read only 
#define	AM_HID	0x02	// Hidden 
#define	AM_SYS	0x04	// System 
#define	AM_VOL	0x08	// Volume label 
#define AM_LFN	0x0F	// LFN entry 
#define AM_DIR	0x10	// Directory 
#define AM_ARC	0x20	// Archive 
#define AM_MASK	0x3F	// Mask of defined bits 



// File system object structure 

uint8_t		fs_csize;		// Number of sectors per cluster N
uint8_t		fs_csize_bits;	// number of bits 2^N
uint8_t		fs_csize_mask;	// 2^N-1 

uint16_t	fs_n_rootdir;	// Number of root directory entries (0 on FAT32) 
uint32_t	fs_n_fatent;	// Number of FAT entries (= number of clusters + 2) 
uint32_t	fs_fatbase;		// FAT start sector 
uint32_t	fs_dirbase;		// Root directory start sector (Cluster# on FAT32) 
uint32_t	fs_database;	// Data start sector 
uint32_t	fs_fsize;		// File size 
uint32_t	fs_org_clust;	// File start cluster 
uint32_t	fs_curr_clust;	// File current cluster 
uint32_t	fs_block;		// Current block, start of cluster 
uint32_t	fs_file_block;	// block index within cluster 

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
// Get sector# from cluster# / Get cluster field from directory entry    
//-----------------------------------------------------------------------

#define clust2sect(X) ((((X) - 2) << fs_csize_bits) + fs_database)


//-----------------------------------------------------------------------
// FAT access - Read value of a FAT entry                                
//-----------------------------------------------------------------------

static uint32_t get_fat (
	uint32_t clst	// Cluster# to get the link information 
)
{
	uint8_t*	buf = disk_read_block1(fs_fatbase + (clst >> 7));
	uint16_t	offset =  (((uint16_t)clst & 127) << 2);

	return ld_dword(&buf[offset]) & 0x0FFFFFFF;
}

//-----------------------------------------------------------------------
// Mount/Unmount a Locical Drive                                         
//-----------------------------------------------------------------------


// ring buffer..

#define QUEUE_SIZE	128
#define QUEUE_MASK (QUEUE_SIZE-1)

uint32_t	RAM_UNITIALIZED queue_block[QUEUE_SIZE];
uint32_t	RAM_UNITIALIZED queue_clust[QUEUE_SIZE];
uint8_t		queue_head = 0;

bool pf_mount()
{
	uint8_t* buf;
	uint32_t bsect, fsize, tsect;

	// Check if the drive is ready or not 
	if (!disk_initialize())
		return false;

	for (int i=0; i<QUEUE_SIZE; i++)
	{
		queue_block[i] = -1;
		queue_clust[i] = -1;
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
	fs_fatbase = bsect + ld_word(buf+BPB_RsvdSecCnt); // FAT start sector (lba) 
	fs_n_rootdir = ld_word(buf+BPB_RootEntCnt);		// Nmuber of root directory entries 


	// Number of sectors per cluster 
	
	fs_csize = buf[BPB_SecPerClus];
	fs_csize_bits = -1;

	while(fs_csize)
	{
		fs_csize_bits++;
		fs_csize >>= 1;
	}

	fs_csize = 1 << fs_csize_bits;
	fs_csize_mask = fs_csize - 1;


	tsect = ld_word(buf+BPB_TotSec16);				// Number of sectors on the file system 
	if (!tsect)
		tsect = ld_dword(buf+BPB_TotSec32);

	// Last cluster# + 1 
	fs_n_fatent = ((tsect - ld_word(buf+BPB_RsvdSecCnt) - fsize - (fs_n_rootdir >> 4)) >> fs_csize_bits) + 2;

	fs_dirbase = ld_dword(buf+(BPB_RootClus));	// Root directory start cluster 
	fs_database = fs_fatbase + fsize + (fs_n_rootdir >> 4);	// Data start sector (lba) 

	// FS_FAT32;
	if (fs_n_fatent >= 0xFFF7)
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

	check = queue_head - 1;
	check &= QUEUE_MASK;

	for (i=0; i<QUEUE_SIZE; i++)
	{
		if (block >= queue_block[check])
		{
			fs_block = queue_block[check];
			fs_curr_clust = queue_clust[check];
			break;
		}

		check--;
		check &= QUEUE_MASK;
	}

	if (fs_block > block)
	{
		fs_block = 0;
		fs_curr_clust = fs_org_clust;
	}

	// Cluster following loop 
	// Follow cluster chain 


	while (block >= (fs_block + fs_csize))
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
			queue_clust[queue_head] =  fs_curr_clust;
			queue_block[queue_head] =  fs_block;
			queue_head++;
			queue_head &= QUEUE_MASK;
		}

		fs_curr_clust = get_fat(fs_curr_clust);
		fs_block += fs_csize;
	}


	// Update start block for this cluster 

	sect = clust2sect(fs_curr_clust);		// Get current sector 
	cs = (uint8_t)(block & fs_csize_mask);	// Sector offset in the cluster 

	fs_file_block = sect + cs;

	return true;
}

//-----------------------------------------------------------------------
// First regular non-deleted file										 
//-----------------------------------------------------------------------

bool pf_open_first(
		uint32_t *numFrames,
		bool	 *numFramesInit
		)
{
	bool		res = true;
	uint8_t		c;
	uint8_t*	buf;
	uint8_t*	dir;

	uint16_t	dir_index = 0;			// Current read/write index number 
	uint32_t	dir_clust = fs_dirbase;	// Current cluster 
	uint32_t	dir_sect = clust2sect(dir_clust);	// Current sector 

	*numFrames = 0;
	*numFramesInit = false;

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
				uint8_t	*b = (uint8_t*)&fs_org_clust;

				// little endian architecture
				b[3] = dir[DIR_FstClusHI + 1];
				b[2] = dir[DIR_FstClusHI + 0];
				b[1] = dir[DIR_FstClusLO + 1];
				b[0] = dir[DIR_FstClusLO + 0];

				fs_fsize = ld_dword(dir+DIR_FileSize);	// File size 
				*numFrames = fs_fsize / (FIELD_NUM_BLOCKS * 512);
				*numFramesInit = true;

				fs_block = 0;						// File pointer 
				fs_curr_clust = fs_org_clust;

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
				if (dir_index >= fs_n_rootdir)
					res = false;	// Report EOT when end of table 
			}
			else // Dynamic table 
			{
				// Cluster changed? 
				if (((dir_index >> 4) & fs_csize_mask) == 0)
				{
					dir_clust = get_fat(dir_clust);		// Get next cluster 
					if (dir_clust >= fs_n_fatent)
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
	disk_read_block2(fs_file_block++, dst);
}

