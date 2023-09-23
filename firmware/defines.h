
/*

   Format description

*/

#ifndef __DEFINES__
#define __DEFINES__

#include <stdint.h>
#include <stdbool.h>

#define FIELD_NUM_BLOCKS	8
#define FIELD_MAX_BLOCKS	6	// pal is 6, but ntsc is only 5, anything larger will require device with more RAM

// 3K
#define FIELD_SIZE			(512*FIELD_MAX_BLOCKS) 

struct frameInfo
{
	uint8_t*  		colorBKBuf;
	uint8_t*  		colorBuf;
	uint8_t*  		graphBuf;
	uint8_t*        audioBuf;

	uint_fast8_t    vsyncLines;
	uint_fast8_t    blankLines;
	uint_fast8_t    overscanLines;
	uint_fast8_t	visibleLines;

	uint8_t*        timecodeBuf;	// not used by kernel
	uint_fast16_t	totalLines;		// not used by kernel
	uint_fast8_t	numBlocks;		// not used by kernel

	bool			odd;
};


#endif
