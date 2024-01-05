/*---------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module include file  R0.03a
/----------------------------------------------------------------------------/
/ Petit FatFs module is an open source software to implement FAT file system to
/ small embedded systems. This is a free software and is opened for education,
/ research and commercial developments under license policy of following trems.
/
/  Copyright (C) 2019, ChaN, all right reserved.
/
/ * The Petit FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial use UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/ Adapted from above for MovieCart use
/----------------------------------------------------------------------------*/

#ifndef PF_DEFINED
#define PF_DEFINED

#include <stdint.h>
#include <stdbool.h>

bool		pf_mount();							/* Mount/Unmount a logical drive */
bool		pf_open_first(uint32_t *numFrames);	/* Open first archived non-deleted file */
bool		pf_seek_block(uint32_t block);		/* Move file pointer of the open file */
void        pf_read_block(uint8_t *dst);		/* Read full block*/

// File system object structure 

struct fileSystemInfo
{
    uint8_t		name[11];	// 8.3 format
        
	uint8_t		csize;		// Number of sectors per cluster N
	uint8_t		csize_bits;	// number of bits 2^N
	uint8_t		csize_mask;	// 2^N-1 

	uint16_t	n_rootdir;	// Number of root directory entries (0 on FAT32) 
	uint32_t	n_fatent;	// Number of FAT entries (= number of clusters + 2) 
	uint32_t	fatbase;		// FAT start sector 
	uint32_t	dirbase;		// Root directory start sector (Cluster# on FAT32) 
	uint32_t	database;	// Data start sector 
	uint32_t	fsize;		// File size 
	uint32_t	org_clust;	// File start cluster 
	uint32_t	curr_clust;	// File current cluster 
	uint32_t	block;		// Current block, start of cluster 
	uint32_t	file_block;	// block index within cluster 


};

#endif /* _PFATFS */
