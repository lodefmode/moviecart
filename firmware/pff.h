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
bool		pf_open_first(uint32_t *numFrames, bool *numFramesInit);	/* Open first archived non-deleted file */
bool		pf_seek_block(uint32_t block);		/* Move file pointer of the open file */
void        pf_read_block(uint8_t *dst);		/* Read full block*/

#endif /* _PFATFS */
