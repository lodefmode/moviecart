
/*

   Frame description

*/

#ifndef __FRAME__
#define __FRAME__

#include <stdint.h>
#include <stdbool.h>

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

	uint8_t*		buffer;
};

extern void frameInit(struct frameInfo* fInfo);
extern void frameInitTitle(struct frameInfo* fInfo, bool odd);

#endif
